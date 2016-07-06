//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Mesh class
// 
//*****************************************************************************

#include "VuGfxSceneMesh.h"
#include "VuGfxSceneMeshPart.h"
#include "VuGfxSceneInfo.h"
#include "VuGfxScene.h"
#include "VuGfxSceneUtil.h"
#include "VuGfxSceneBakeState.h"

#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuVertexBuffer.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/Math/VuMathUtil.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuEndianUtil.h"
#include "VuEngine/Util/VuVertexCacheOpt.h"


class VuMeshPart
{
public:
	VuMeshPart() : mTriCount(0), mVertCount(0), mMinIndex(0xffff), mMaxIndex(0), mStartIndex(0) {}

	VuArray<VUUINT16>	mIndices;
	VuArray<VUBYTE>		mVerts;
	int					mTriCount;
	int					mVertCount;
	int					mMinIndex;
	int					mMaxIndex;
	int					mStartIndex;
};


//*****************************************************************************
VuGfxSceneMesh::VuGfxSceneMesh()
{
}

//*****************************************************************************
VuGfxSceneMesh::~VuGfxSceneMesh()
{
	// release mesh parts
	for ( Parts::iterator iter = mParts.begin(); iter != mParts.end(); iter++ )
		(*iter)->removeRef();
}

//*****************************************************************************
void VuGfxSceneMesh::load(VuBinaryDataReader &reader)
{
	// name
	reader.readString(mstrName);

	// parts
	int partCount;
	reader.readValue(partCount);
	mParts.resize(partCount);
	for ( Parts::iterator iter = mParts.begin(); iter != mParts.end(); iter++ )
	{
		*iter = new VuGfxSceneMeshPart;
		(*iter)->load(reader);
	}

	// misc
	reader.readValue(mAabb);
}

//*****************************************************************************
bool VuGfxSceneMesh::bake(const VuJsonContainer &data, int meshIndex, VuGfxSceneBakeState &bakeState, int vertexStride, bool bSkinning, bool flipX, VuBinaryDataWriter &writer)
{
	// name
	std::string name = data["Name"].asString();
	writer.writeString(name);

	bakeState.mMeshLookup[name] = meshIndex;

	// read indices
	VuArray<VUUINT16> rawIndices(0);
	{
		const VuJsonContainer &indices = data["Indices"];
		VuArray<VUBYTE> bytes;
		if ( !VuDataUtil::getValue(indices["Data"], bytes) )
			return VUWARNING("Unable to load index data for mesh %s", name.c_str());

		if ( bytes.size() != indices["IndexCount"].asInt()*4 )
			return VUWARNING("IndexCount issue for mesh %s", name.c_str());

		int indexCount = bytes.size()/4;

		#if VU_BIG_ENDIAN
		{
			VUUINT32 *p = (VUUINT32 *)&bytes.begin();
			for ( int i = 0; i < indexCount; i++ )
				VuEndianUtil::swapInPlace<VUUINT32>(*p++);
		}
		#endif

		// add to index buffer
		rawIndices.resize(indexCount);
		VUUINT32 *pSrc = (VUUINT32 *)&bytes.begin();
		VUUINT16 *pDst = &rawIndices[0];
		for ( int i = 0; i < indexCount; i++ )
		{
			int index = *pSrc;
			if ( index > 0xffff )
				return VUWARNING("Index out of range (65535 max) for mesh %s", name.c_str());
			*pDst = (VUUINT16)index;
			pSrc++;
			pDst++;
		}
	}

	// mesh elements
	VuVertexDeclarationElements meshElements;
	meshElements.load(data["VertexDeclaration"]);

	// read verts
	VuArray<VUBYTE> rawVerts(0);
	if ( !VuDataUtil::getValue(data["Verts"]["Data"], rawVerts) )
		return VUWARNING("Unable to load vertex data for mesh %s", name.c_str());

	int rawVertexSize = meshElements.calcVertexSize(0);
//	int rawVertexCount = rawVerts.size()/rawVertexSize;

	// build verts/inds for each part
	const VuJsonContainer &parts = data["Parts"];
	int partCount = parts.size();
	VuMeshPart *pMeshParts = new VuMeshPart[partCount];

	for ( int iPart = 0; iPart < partCount; iPart++ )
	{
		VuMeshPart *pPart = &pMeshParts[iPart];
		const VuJsonContainer &partData = parts[iPart];

		pPart->mTriCount = partData["TriCount"].asInt();
		int startIndex = partData["StartIndex"].asInt();

		for ( int rawIndex = 0; rawIndex < pPart->mTriCount*3; rawIndex++ )
		{
			VUBYTE *pRawVert = &rawVerts[rawIndices[startIndex + rawIndex]*rawVertexSize];

			// try to find vertex
			int index = 0;
			for ( VUBYTE *pVert = &pPart->mVerts.begin(); pVert < &pPart->mVerts.end(); pVert += rawVertexSize )
			{
				if ( memcmp(pVert, pRawVert, rawVertexSize) == 0 )
					break;
				index++;
			}

			// if not found, add the vert
			if ( index == pPart->mVertCount )
			{
				pPart->mVerts.resize((pPart->mVertCount + 1)*rawVertexSize);
				VU_MEMCPY(&pPart->mVerts[pPart->mVertCount*rawVertexSize], rawVertexSize, pRawVert, rawVertexSize);
				pPart->mVertCount++;
			}

			if ( index > 0xffff )
				return VUWARNING("Index out of range (65535 max) for mesh %s", name.c_str());

			// add the index
			pPart->mIndices.push_back((VUUINT16)index);

			pPart->mMinIndex = VuMin(pPart->mMinIndex, index);
			pPart->mMaxIndex = VuMax(pPart->mMaxIndex, index);
		}
	}

	// flip mesh
	if ( flipX )
	{
		for ( int iPart = 0; iPart < partCount; iPart++ )
		{
			VuMeshPart *pPart = &pMeshParts[iPart];
//			const VuJsonContainer &partData = parts[iPart];

			// reverse indices
			for ( int i = 0; i < pPart->mTriCount; i++ )
				VuSwap(pPart->mIndices[i*3 + 0], pPart->mIndices[i*3 + 2]);

			// flip verts
			for ( int iVert = 0; iVert < pPart->mVertCount; iVert++ )
			{
				for ( int iElement = 0; iElement < (int)meshElements.size(); iElement++ )
				{
					const VuVertexDeclarationElement &element = meshElements[iElement];

					if ( element.mType == VUGFX_DECL_TYPE_FLOAT3 )
					{
						if ( element.mUsage == VUGFX_DECL_USAGE_POSITION || element.mUsage == VUGFX_DECL_USAGE_NORMAL || element.mUsage == VUGFX_DECL_USAGE_TANGENT )
						{
							float *pVec3 = (float *)(&pPart->mVerts[iVert*rawVertexSize + element.mOffset]);
							pVec3[0] *= -1.0f;
						}
					}
				}
			}
		}
	}

	// optimize indices
	for ( int iPart = 0; iPart < partCount; iPart++ )
	{
		VuMeshPart *pPart = &pMeshParts[iPart];
		const VuJsonContainer &partData = parts[iPart];
		const std::string &materialName = partData["Material"].asString();
		VuGfxSceneBakeState::Chunk &chunk = bakeState.mChunks[bakeState.mChunkLookup[materialName]];

		int partVertexOffset = chunk.mPartVertexOffset;
		int partIndexOffset = (int)chunk.mIndexData.size();

		// optimize indices for vertex cache
		VuArray<VUUINT16> temp(0);
		temp.resize(pPart->mTriCount*3);
		Forsyth::OptimizeFaces(&pPart->mIndices[0], pPart->mTriCount*3, pPart->mMaxIndex + 1, &temp[0]);
		VU_MEMCPY(&pPart->mIndices[0], pPart->mIndices.size()*sizeof(VUUINT16), &temp[0], pPart->mTriCount*3*sizeof(VUUINT16));

		// add vertex offset
		for ( int i = 0; i < pPart->mTriCount*3; i++ )
		{
			int index = partVertexOffset + pPart->mIndices[i];
			if ( index > 0xffff )
				return VUWARNING("Index out of range (65535 max) for mesh %s", name.c_str());
			pPart->mIndices[i] = (VUUINT16)index;
		}
		pPart->mMinIndex += partVertexOffset;
		pPart->mMaxIndex += partVertexOffset;
		pPart->mStartIndex = partIndexOffset;

		chunk.mPartVertexOffset += pPart->mVertCount;

		// write out indices
		chunk.mIndexData.resize(chunk.mIndexData.size() + pPart->mIndices.size());
		VU_MEMCPY(&chunk.mIndexData[pPart->mStartIndex], (chunk.mIndexData.size() - pPart->mStartIndex)*sizeof(VUUINT16), &pPart->mIndices[0], pPart->mTriCount*3*sizeof(VUUINT16));
	}

	// optimize verts
	for ( int iPart = 0; iPart < partCount; iPart++ )
	{
		VuMeshPart *pPart = &pMeshParts[iPart];
		const VuJsonContainer &partData = parts[iPart];
		const std::string &materialName = partData["Material"].asString();
		VuGfxSceneBakeState::Chunk &chunk = bakeState.mChunks[bakeState.mChunkLookup[materialName]];

		// determine mapping from mesh elements to shader elements
		int elementMap[256];
		for ( int iElement = 0; iElement < (int)chunk.mShaderElements.size(); iElement++ )
		{
			const VuVertexDeclarationElement &element = chunk.mShaderElements[iElement];

			// find in mesh vertex declaration
			elementMap[iElement] = -1;

			// if this element is not found in  vertex declaration, skip it
			for ( int i = 0; i < (int)meshElements.size(); i++ )
				if ( element.mUsage == meshElements[i].mUsage && element.mUsageIndex == meshElements[i].mUsageIndex )
					elementMap[iElement] = i;
		}

		// convert verts to optimized format
		VuArray<VUBYTE> partVerts(0);
		partVerts.resize(pPart->mVertCount*vertexStride);
		memset(&partVerts[0], 0, partVerts.size());
		for ( int iVert = 0; iVert < pPart->mVertCount; iVert++ )
		{
			VUBYTE *pDst = &partVerts[iVert*vertexStride];

			for ( int iElement = 0; iElement < (int)chunk.mShaderElements.size(); iElement++ )
			{
				const VuVertexDeclarationElement &dstElement = chunk.mShaderElements[iElement];
				int dstSize = dstElement.size();

				if ( elementMap[iElement] >= 0 )
				{
					// exists in mesh
					const VuVertexDeclarationElement &srcElement = meshElements[elementMap[iElement]];
					int srcSize = srcElement.size();
					VUBYTE *pSrc = &pPart->mVerts[iVert*rawVertexSize + srcElement.mOffset];

					if ( srcElement.mType == dstElement.mType )
					{
						memmove(pDst, pSrc, srcSize);
					}
					else if ( srcElement.mType == VUGFX_DECL_TYPE_FLOAT2 && dstElement.mType == VUGFX_DECL_TYPE_FLOAT16_2 )
					{
						float *pSrcVal = (float *)pSrc;
						VUUINT16 *pDstVal = (VUUINT16 *)pDst;
						pDstVal[0] = VuMathUtil::floatToHalf(pSrcVal[0]);
						pDstVal[1] = VuMathUtil::floatToHalf(pSrcVal[1]);
					}
					else if ( srcElement.mType == VUGFX_DECL_TYPE_FLOAT3 && dstElement.mType == VUGFX_DECL_TYPE_BYTE4N )
					{
						float *pSrcVal = (float *)pSrc;
						VUINT8 *pDstVal = (VUINT8 *)pDst;
						pDstVal[0] = (VUINT8)VuRound(127.0f*VuClamp(pSrcVal[0], -1.0f, 1.0f));
						pDstVal[1] = (VUINT8)VuRound(127.0f*VuClamp(pSrcVal[1], -1.0f, 1.0f));
						pDstVal[2] = (VUINT8)VuRound(127.0f*VuClamp(pSrcVal[2], -1.0f, 1.0f));
						pDstVal[3] = 0;
					}
					else if ( srcElement.mType == VUGFX_DECL_TYPE_FLOAT3 && dstElement.mType == VUGFX_DECL_TYPE_UBYTE4N )
					{
						float *pSrcVal = (float *)pSrc;
						VUUINT8 *pDstVal = (VUUINT8 *)pDst;

						if ( dstElement.mUsage == VUGFX_DECL_USAGE_BLENDWEIGHT )
						{
							// make sure that any precision rounding errors are absorbed by the bone with the most weight

							// calculate floating-point weights (0-255)
							float fWeights[4];
							fWeights[0] = 255.0f*VuClamp(pSrcVal[0], 0.0f, 1.0f);
							fWeights[1] = 255.0f*VuClamp(pSrcVal[1], 0.0f, 1.0f);
							fWeights[2] = 255.0f*VuClamp(pSrcVal[2], 0.0f, 1.0f);
							fWeights[3] = 255.0f - (fWeights[0] + fWeights[1] + fWeights[2]);

							// round to integer weights
							int iWeights[4];
							for ( int i = 0; i < 4; i++ )
								iWeights[i] = VuRound(fWeights[i]);

							// check if total weight is 255
							if ( iWeights[0] + iWeights[1] + iWeights[2] + iWeights[3] != 255 )
							{
								// apply precision error to bone with most weight (ignore bone 3 because it won't be written out)
								if ( iWeights[0] >= iWeights[1] && iWeights[0] >= iWeights[2] )
									iWeights[0] = 255 - iWeights[1] - iWeights[2];
								else if ( iWeights[1] >= iWeights[2] )
									iWeights[1] = 255 - iWeights[0] - iWeights[2];
								else
									iWeights[2] = 255 - iWeights[0] - iWeights[1];
							}

							pDstVal[0] = (VUUINT8)iWeights[0];
							pDstVal[1] = (VUUINT8)iWeights[1];
							pDstVal[2] = (VUUINT8)iWeights[2];
							pDstVal[3] = 0;
						}
						else
						{
							pDstVal[0] = (VUUINT8)VuRound(255.0f*VuClamp(pSrcVal[0], 0.0f, 1.0f));
							pDstVal[1] = (VUUINT8)VuRound(255.0f*VuClamp(pSrcVal[1], 0.0f, 1.0f));
							pDstVal[2] = (VUUINT8)VuRound(255.0f*VuClamp(pSrcVal[2], 0.0f, 1.0f));
							pDstVal[3] = 0;
						}
					}
					else if ( srcElement.mType == VUGFX_DECL_TYPE_FLOAT3 && dstElement.mType == VUGFX_DECL_TYPE_SHORT4N )
					{
						float *pSrcVal = (float *)pSrc;
						VUINT16 *pDstVal = (VUINT16 *)pDst;
						pDstVal[0] = (VUINT16)VuRound(32767.0f*VuClamp(pSrcVal[0], -1.0f, 1.0f));
						pDstVal[1] = (VUINT16)VuRound(32767.0f*VuClamp(pSrcVal[1], -1.0f, 1.0f));
						pDstVal[2] = (VUINT16)VuRound(32767.0f*VuClamp(pSrcVal[2], -1.0f, 1.0f));
						pDstVal[3] = 0;
					}
					else
					{
						VUASSERT(0, "VuGfxScene::optimizeVerts() unsupported conversion");
					}
				}
				else
				{
					// does not exist in mesh, set to 0
					memset(pDst, 0, dstSize);
				}

				pDst += dstSize;
			}
		}

		// platform endianness
		if ( writer.getSwapEndian() )
		{
			VuVertexBuffer::endianSwap(&partVerts[0], pPart->mVertCount, vertexStride, chunk.mShaderElements);
		}

		// write
		int oldSize = (int)chunk.mVertexData.size();
		chunk.mVertexData.resize(oldSize + partVerts.size());
		VU_MEMCPY(&chunk.mVertexData[oldSize], partVerts.size(), &partVerts[0], partVerts.size());
	}

	// aabb
	VuAabb aabb;
	if ( !VuDataUtil::getValue(data["Verts"]["Min"], aabb.mMin) || !VuDataUtil::getValue(data["Verts"]["Max"], aabb.mMax) )
		return VUWARNING("Unable to load aabb for vertex buffer.");
	if ( flipX )
	{
		aabb.mMin.mX *= -1.0f;
		aabb.mMax.mX *= -1.0f;
	}

	// parts
	writer.writeValue(partCount);
	int vertexCount = 0;
	for ( int iPart = 0; iPart < parts.size(); iPart++ )
	{
		VuMeshPart *pPart = &pMeshParts[iPart];
		const VuJsonContainer &partData = parts[iPart];
		const std::string &materialName = partData["Material"].asString();
		int chunkIndex = bakeState.mChunkLookup[materialName];
		int materialIndex = bakeState.mMaterialLookup[materialName];

		VuAabb aabb;
		VuDataUtil::getValue(partData["Aabb"], aabb);
		if ( flipX )
		{
			aabb.mMin.mX *= -1.0f;
			aabb.mMax.mX *= -1.0f;
		}

		writer.writeValue(chunkIndex);
		writer.writeValue(materialIndex);
		writer.writeValue(pPart->mMinIndex);
		writer.writeValue(pPart->mMaxIndex);
		writer.writeValue(pPart->mStartIndex);
		writer.writeValue(pPart->mTriCount);
		writer.writeValue(aabb);

		vertexCount += pPart->mVertCount;
	}

	// misc
	writer.writeValue(aabb);

	// clean up
	delete[] pMeshParts;

	return true;
}

//*****************************************************************************
bool VuGfxSceneMesh::fixup(const VuGfxScene *pScene)
{
	// fixup parts
	for ( Parts::iterator iter = mParts.begin(); iter != mParts.end(); iter++ )
		if ( !(*iter)->fixup(pScene) )
			return VUWARNING("Unable to fix up mesh '%s'.", mstrName.c_str());

	return true;
}

//*****************************************************************************
void VuGfxSceneMesh::gatherSceneInfo(VuGfxSceneInfo &sceneInfo)
{
	// mesh info
	sceneInfo.mNumMeshes++;

	// part info
	for ( Parts::iterator iter = mParts.begin(); iter != mParts.end(); iter++ )
		(*iter)->gatherSceneInfo(sceneInfo);
}