//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Scene class
// 
//*****************************************************************************

#include "VuGfxScene.h"
#include "VuGfxSceneMesh.h"
#include "VuGfxSceneMeshPart.h"
#include "VuGfxSceneChunk.h"
#include "VuGfxSceneMaterial.h"
#include "VuGfxSceneBakeState.h"
#include "VuGfxSceneUtil.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuVertexBuffer.h"
#include "VuEngine/HAL/Gfx/VuIndexBuffer.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAssetBakery.h"
#include "VuEngine/Assets/VuMaterialAsset.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Math/VuMathUtil.h"


#define MAX_VERTEX_STRIDE 128


//*****************************************************************************
VuGfxScene::VuGfxScene()
{
}

//*****************************************************************************
void VuGfxScene::clear()
{
	// release materials
	for ( Materials::iterator iter = mMaterials.begin(); iter != mMaterials.end(); iter++ )
		(*iter)->removeRef();
	mMaterials.clear();

	// release meshes
	for ( Meshes::iterator iter = mMeshes.begin(); iter != mMeshes.end(); iter++ )
		(*iter)->removeRef();
	mMeshes.clear();

	// release chunks
	for ( Chunks::iterator iter = mChunks.begin(); iter != mChunks.end(); iter++ )
		(*iter)->removeRef();
	mChunks.clear();
}

//*****************************************************************************
bool VuGfxScene::load(VuBinaryDataReader &reader)
{
	// materials
	int materialCount;
	reader.readValue(materialCount);
	mMaterials.resize(materialCount);
	for ( int iMaterial = 0; iMaterial < materialCount; iMaterial++ )
	{
		mMaterials[iMaterial] = new VuGfxSceneMaterial(iMaterial);
		if ( !mMaterials[iMaterial]->load(reader) )
			return false;
	}

	// meshes
	int meshCount;
	reader.readValue(meshCount);
	mMeshes.resize(meshCount);
	for ( int iMesh = 0; iMesh < meshCount; iMesh++ )
	{
		mMeshes[iMesh] = new VuGfxSceneMesh;
		mMeshes[iMesh]->load(reader);
	}

	// chunks
	int chunkCount;
	reader.readValue(chunkCount);
	mChunks.resize(chunkCount);
	for ( int iChunk = 0; iChunk < chunkCount; iChunk++ )
	{
		mChunks[iChunk] = new VuGfxSceneChunk(iChunk);
		mChunks[iChunk]->load(reader);
	}

	// fixup
	for ( Meshes::iterator iter = mMeshes.begin(); iter != mMeshes.end(); iter++ )
		(*iter)->fixup(this);

	// gather info
	gatherSceneInfo();

	return true;
}

//*****************************************************************************
bool VuGfxScene::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams, const VuJsonContainer &data, VuGfxSceneBakeState &bakeState, bool bSkinning, bool flipX, VuBinaryDataWriter &writer)
{
	VuJsonContainer materials;
	VuGfxSceneUtil::cleanUpMaterials(creationInfo, data, materials, bSkinning ? "DefaultAnimated" : "Default");

	// gather chunks
	int materialCount = materials.size();
	for ( int iMaterial = 0; iMaterial < materialCount; iMaterial++ )
	{
		const VuJsonContainer &material = materials[iMaterial];

		const std::string &materialName = material["Name"].asString();

		// get shader file name
		std::string shaderFileName;
		{
			const std::string &materialAssetName = material["MaterialAsset"].asString();
			const VuJsonContainer &assetInfo = VuAssetBakery::IF()->getCreationInfo(bakeParams.mPlatform, bakeParams.mSku, bakeParams.mLanguage, VuMaterialAsset::msRTTI.mstrType, materialAssetName);
			shaderFileName = assetInfo["File"].asString();
			if ( shaderFileName.empty() )
				shaderFileName = VuAssetBakery::IF()->getCreationInfo(VUPLATFORM, VuAssetFactory::IF()->getSku(), VuSys::IF()->getLanguage(), VuMaterialAsset::msRTTI.mstrType, "Default")["File"].asString();
		}

		if ( bakeState.chunkIndex(shaderFileName) == -1 )
		{
			VuGfxSceneBakeState::Chunk chunk;

			// get optimized shader elements
			VuVertexDeclarationElements optElements;
			{
				VuJsonReader reader;
				VuJsonContainer shaderData;
				if ( !reader.loadFromFile(shaderData, VuFile::IF()->getRootPath() + shaderFileName) )
					return VUWARNING("Unable to load shader asset '%s': %s", shaderFileName.c_str(), reader.getLastError().c_str());

				// vertex declaration
				VuVertexDeclarationElements elements;
				elements.load(shaderData["VertexDeclaration"]);
				VuGfxSceneUtil::optimizeVertexDeclaration(bakeParams.mPlatform, bSkinning, elements, optElements);
			}

			chunk.mShaderFileName = shaderFileName;
			chunk.mShaderElements = optElements;
			chunk.mVertexStride = chunk.mShaderElements.calcVertexSize(0);

			bakeState.mChunks.push_back(chunk);
		}

		bakeState.mChunkLookup[materialName] = bakeState.chunkIndex(shaderFileName);
		bakeState.mMaterialLookup[materialName] = iMaterial;
	}

	// materials
	writer.writeValue(materialCount);
	for ( int iMaterial = 0; iMaterial < materialCount; iMaterial++ )
		if ( !VuGfxSceneMaterial::bake(materials[iMaterial], writer, bakeParams.mDependencies) )
			return false;

	// meshes
	const VuJsonContainer &meshes = data["Meshes"];
	int meshCount = meshes.size();
	writer.writeValue(meshCount);
	for ( int iMesh = 0; iMesh < meshes.size(); iMesh++ )
		if ( !VuGfxSceneMesh::bake(meshes[iMesh], iMesh, bakeState, MAX_VERTEX_STRIDE, bSkinning, flipX, writer) )
			return false;

	// chunks
	int chunkCount = (int)bakeState.mChunks.size();
	writer.writeValue(chunkCount);
	for ( int i = 0; i < (int)bakeState.mChunks.size(); i++ )
	{
		VuGfxSceneBakeState::Chunk &chunk = bakeState.mChunks[i];

		// make sure we have some vertex data
		if ( chunk.mVertexData.size() == 0 )
			return VUWARNING("Empty scene.");

		// optimize verts
		optimizeVerts(chunk.mVertexData, MAX_VERTEX_STRIDE, chunk.mVertexStride);

		VuGfxSceneChunk::bake(chunk, writer);
	}


	return true;
}

//*****************************************************************************
void VuGfxScene::optimizeVerts(std::vector<VUBYTE> &vertexData, int oldStride, int newStride)
{
	int vertCount = (int)vertexData.size()/oldStride;

	// reduce vertex stride
	{
		VUBYTE *pSrc = &vertexData[0];
		VUBYTE *pDst = pSrc;
		for ( int iVert = 0; iVert < vertCount; iVert++ )
		{
			memmove(pDst, pSrc, newStride);
			pSrc += oldStride;
			pDst += newStride;
		}
		vertexData.resize(vertCount*newStride);
	}
}

//*****************************************************************************
void VuGfxScene::gatherSceneInfo()
{
	for ( Chunks::const_iterator iter = mChunks.begin(); iter != mChunks.end(); iter++ )
	{
		mInfo.mNumTris += (*iter)->mpIndexBuffer->getIndexCount()/3;
		mInfo.mNumVerts += (*iter)->mpVertexBuffer->mSize/(*iter)->mVertexStride;
	}

	// material info
	for ( Materials::const_iterator iter = mMaterials.begin(); iter != mMaterials.end(); iter++ )
		mInfo.mNumMaterials++;

	// mesh info
	for ( Meshes::const_iterator iter = mMeshes.begin(); iter != mMeshes.end(); iter++ )
		(*iter)->gatherSceneInfo(mInfo);
}
