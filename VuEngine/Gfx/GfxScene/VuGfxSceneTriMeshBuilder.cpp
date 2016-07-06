//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GfxSceneTriMeshBuilder class
// 
//*****************************************************************************

#include "VuGfxSceneTriMeshBuilder.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneUtil.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAssetBakery.h"
#include "VuEngine/Assets/VuCollisionMaterialAsset.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuEndianUtil.h"


//*****************************************************************************
VuGfxSceneTriMeshBuilder::VuGfxSceneTriMeshBuilder(const std::string &platform, const std::string &sku, const std::string &language, const VuJsonContainer &creationInfo, const VuJsonContainer &data):
	mPlatform(platform),
	mSku(sku),
	mLanguage(language),
	mCreationInfo(creationInfo),
	mData(data)
{
}

//*****************************************************************************
VuGfxSceneTriMeshBuilder::~VuGfxSceneTriMeshBuilder()
{
}

//*****************************************************************************
void VuGfxSceneTriMeshBuilder::build(bool flipX)
{
	// gather materials
	std::set<std::string> sceneMaterials;
	VuGfxSceneUtil::gatherSceneMaterialNames(mData, sceneMaterials);

	mMaterials.resize(sceneMaterials.size());

	int iMat = 0;
	for ( std::set<std::string>::const_iterator iter = sceneMaterials.begin(); iter != sceneMaterials.end(); iter++ )
	{
		Material &material = mMaterials[iMat];

		material.mName = *iter;
		material.mAssetName = mCreationInfo[*iter].asString();

		const VuJsonContainer &assetInfo = VuAssetBakery::IF()->getCreationInfo(mPlatform, mSku, mLanguage, VuCollisionMaterialAsset::msRTTI.mstrType, material.mAssetName);
		if ( assetInfo.isNull() )
			material.mAssetName = "None";

		material.mSurfaceType = "<none>";
		material.mHardEdgeThreshold = 30.0f;
		material.mCoronaCollision = false;
		material.mReceiveShadows = false;
		material.mIgnoreBakedShadows = false;

		VuDataUtil::getValue(assetInfo["Surface Type"], material.mSurfaceType);
		VuDataUtil::getValue(assetInfo["Hard Edge Threshold"], material.mHardEdgeThreshold);
		VuDataUtil::getValue(assetInfo["Corona Collision"], material.mCoronaCollision);
		VuDataUtil::getValue(assetInfo["Receive Shadows"], material.mReceiveShadows);
		VuDataUtil::getValue(assetInfo["Ignore Baked Shadows"], material.mIgnoreBakedShadows);

		iMat++;
	}

	mVerts.clear();
	mIndices.clear();

	// build meshes
	const VuJsonContainer &meshesData = mData["Meshes"];
	for ( int iMesh = 0; iMesh < (int)meshesData.size(); iMesh++ )
	{
		const VuJsonContainer &meshData = meshesData[iMesh];
		buildMesh(meshData, mMeshes[meshData["Name"].asString()]);
	}

	// handle nodes
	const VuJsonContainer &nodes = mData["Nodes"];
	for ( int iNode = 0; iNode < nodes.size(); iNode++ )
		gatherTrisRecursive(nodes[iNode], VuMatrix::identity());

	// copy indices
	for ( int iMat = 0; iMat < (int)mMaterials.size(); iMat++ )
	{
		const Material &material = mMaterials[iMat];

		for ( int i = 0; i < material.mIndices.size(); i++ )
			mIndices.push_back(material.mIndices[i]);

		for ( int i = 0; i < material.mTris.size(); i++ )
			mTris.push_back(material.mTris[i]);
	}

	// flip
	if ( flipX )
	{
		// reverse indices
		for ( int i = 0; i < mTris.size(); i++ )
			VuSwap(mIndices[i*3 + 0], mIndices[i*3 + 2]);

		// flip verts
		for ( int i = 0; i < mVerts.size(); i++ )
			mVerts[i].mX *= -1.0f;
	}
}

//*****************************************************************************
void VuGfxSceneTriMeshBuilder::buildMesh(const VuJsonContainer &data, Mesh &mesh)
{
	// get parts data
	mesh.mpPartsData = &data["Parts"];

	// verts
	{
		// mesh elements
		VuVertexDeclarationElements elements;
		elements.load(data["VertexDeclaration"]);

		int colOffset = -1;
		int offset = 0;
		for ( int i = 0; i < (int)elements.size(); i++ )
		{
			if ( elements[i].mUsage == VUGFX_DECL_USAGE_COLOR )
				colOffset = offset;
			offset += elements[i].size();
		}

		VuArray<VUBYTE> bytes;
		if ( VuDataUtil::getValue(data["Verts"]["Data"], bytes) )
		{
			int vertexCount = data["NumVerts"].asInt();
			int vertexSize = data["VertexSize"].asInt();

			if ( bytes.size() == vertexCount*vertexSize )
			{
				mesh.mVerts.resize(vertexCount);
				mesh.mColors.resize(vertexCount);

				// assume position is first float-triplet
				for ( int iVert = 0; iVert < vertexCount; iVert++ )
				{
					// assume position is first float-triplet
					float *pPos = (float *)(&bytes[iVert*vertexSize]);
					mesh.mVerts[iVert].mX = pPos[0];
					mesh.mVerts[iVert].mY = pPos[1];
					mesh.mVerts[iVert].mZ = pPos[2];

					if ( colOffset >= 0 )
						mesh.mColors[iVert] = *(VuColor *)(&bytes[iVert*vertexSize + colOffset]);
					else
						mesh.mColors[iVert] = VuColor(255,255,255);

					// fix endianness
					#if VU_BIG_ENDIAN
						VuEndianUtil::swap<float>(mVerts[iVert].mX);
						VuEndianUtil::swap<float>(mVerts[iVert].mY);
						VuEndianUtil::swap<float>(mVerts[iVert].mZ);
						VuEndianUtil::swap<VuColor>(mesh.mColors[iVert]);
					#endif
				}
			}
		}
	}

	// indices
	{
		VuArray<VUBYTE> bytes;
		if ( VuDataUtil::getValue(data["Indices"]["Data"], bytes) )
		{
			int indexCount = data["Indices"]["IndexCount"].asInt();
			VUASSERT(indexCount == bytes.size()/4, "VuGfxSceneTriMeshBuilder::buildMesh() index size error");

			// fix endianness
			#if VU_BIG_ENDIAN
			{
				VUUINT32 *p = (VUUINT32 *)&bytes.begin();
				for ( int i = 0; i < indexCount; i++ )
					VuEndianUtil::swapInPlace<VUUINT32>(*p++);
			}
			#endif

			mesh.mIndices.resize(indexCount);
			VU_MEMCPY(&mesh.mIndices[0], indexCount*sizeof(int), &bytes.begin(), bytes.size());
		}
	}
}

//*****************************************************************************
void VuGfxSceneTriMeshBuilder::gatherTrisRecursive(const VuJsonContainer &data, const VuMatrix &transform)
{
	// handle transform
	VuMatrix nodeTransform;
	nodeTransform.loadIdentity();
	VuDataUtil::getValue(data["Transform"], nodeTransform);
	nodeTransform *= transform;

	// mesh instance
	const VuJsonContainer &meshInstData = data["MeshInstance"];
	Meshes::const_iterator itMesh = mMeshes.find(meshInstData["Mesh"].asString());
	if ( itMesh != mMeshes.end() )
	{
		// parts
		const VuJsonContainer &partsData = *itMesh->second.mpPartsData;
		for ( int iPart = 0; iPart < partsData.size(); iPart++ )
		{
			// part
			const VuJsonContainer &partData = partsData[iPart];
			addTris(partData, itMesh->second, nodeTransform);
		}
	}

	// nodes (recurse)
	const VuJsonContainer &nodes = data["Nodes"];
	for ( int iNode = 0; iNode < nodes.size(); iNode++ )
		gatherTrisRecursive(nodes[iNode], nodeTransform);
}

//*****************************************************************************
void VuGfxSceneTriMeshBuilder::addTris(const VuJsonContainer &data, const Mesh &mesh, const VuMatrix &transform)
{
	const std::string &materialName = data["Material"].asString();

	// determine shader index
	int materialIndex;
	for ( materialIndex = 0; materialIndex < mMaterials.size(); materialIndex++ )
		if ( mMaterials[materialIndex].mName == materialName )
			break;

	// if not found in our applicable shader list, don't add tris
	if ( materialIndex == mMaterials.size() )
		return;

	int startIndex = data["StartIndex"].asInt();
	int triCount = data["TriCount"].asInt();
	int indexCount = triCount*3;

	// add verts/indices
	for ( int i = 0; i < indexCount; i++ )
	{
		// get vert from mesh
		int index = mesh.mIndices[startIndex + i];
		VuVector3 vert = mesh.mVerts[index];

		// apply transform
		vert = transform.transform(vert);

		// add vert
		for ( index = 0; index < (int)mVerts.size(); index++ )
			if ( mVerts[index] == vert )
				break;

		if ( index == mVerts.size() )
			mVerts.push_back(vert);
		mMaterials[materialIndex].mIndices.push_back(index);
	}

	// add shader indices
	for ( int i = 0; i < triCount; i++ )
	{
		int index0 = mesh.mIndices[startIndex + i*3 + 0];
		int index1 = mesh.mIndices[startIndex + i*3 + 1];
		int index2 = mesh.mIndices[startIndex + i*3 + 2];

		Triangle tri;
		tri.mColor0 = mesh.mColors[index0];
		tri.mColor1 = mesh.mColors[index1];
		tri.mColor2 = mesh.mColors[index2];
		tri.mMaterialIndex = materialIndex;
		mMaterials[materialIndex].mTris.push_back(tri);
	}
}
