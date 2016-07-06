//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Collision Mesh Asset class
// 
//*****************************************************************************

#include "VuCollisionMeshAsset.h"
#include "VuCollisionMaterialAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneTriMeshBuilder.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/Shaders/VuCollisionShader.h"
#include "VuEngine/Dynamics/VuDynamics.h"
#include "VuEngine/Dynamics/VuStridingMesh.h"
#include "VuEngine/Dynamics/VuOptimizedBvh.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Memory/VuScratchPad.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Math/VuMathUtil.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Dev/VuDevUtil.h"


IMPLEMENT_RTTI(VuCollisionMeshAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuCollisionMeshAsset);

static bool sDrawEdges = false;

#define EDGE_COLLISION_DISTANCE 0.01f // m


//*****************************************************************************
VuCollisionMeshAsset::VuCollisionMeshAsset():
	mVerts(0),
	mIndices(0),
	mTriangles(0),
	mMaterials(0),
	mpBvh(VUNULL)
{
	static VuDevBoolOnce sbOnce;
	if ( sbOnce && VuDevMenu::IF() )
		VuDevMenu::IF()->addBool("Collision/DrawEdges", sDrawEdges);
}

//*****************************************************************************
btOptimizedBvh *VuCollisionMeshAsset::getBvh() const
{
	return mpBvh;
}

//*****************************************************************************
void VuCollisionMeshAsset::draw(const VuColor &color, const VuMatrix &modelMat) const
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			VuGfxUtil::IF()->collisionShader()->setConstants(pData->mModelMat, pData->mColor);
			VuGfx::IF()->drawIndexedPrimitiveUP(
				VUGFX_PT_TRIANGLELIST,				// PrimitiveType
				0,									// MinVertexIndex
				pData->mpAsset->getVertCount(),		// NumVertices
				pData->mpAsset->getIndexCount()/3,	// PrimitiveCount
				&pData->mpAsset->getIndex(0),		// IndexData
				&pData->mpAsset->getVert(0)			// VertexStreamZeroData
			);
		}

		VuMatrix					mModelMat;
		VuColor						mColor;
		const VuCollisionMeshAsset	*mpAsset;
	};

	if ( VuGfxUtil::IF()->collisionShader()->getMaterial() == VUNULL )
		return;

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelMat = modelMat;
	pData->mColor = color;
	pData->mpAsset = this;

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, VuGfxUtil::IF()->collisionShader()->getMaterial(), VUNULL, &DrawData::callback);

	if ( sDrawEdges )
		drawEdges(modelMat);
}

//*****************************************************************************
void VuCollisionMeshAsset::drawWithColors(const VuMatrix &modelMat) const
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			for ( int iMat = 0; iMat < pData->mpAsset->getMaterialCount(); iMat++ )
			{
				const VuCollisionMeshAsset::VuMaterial &material = pData->mpAsset->getMaterial(iMat);
				const VuColor &color = VuDynamics::IF()->getSurfaceColor(material.mSurfaceTypeID);

				if ( material.mFlags & VuMaterial::IS_USED )
				{
					VuGfxUtil::IF()->collisionShader()->setConstants(pData->mModelMat, color);
					VuGfx::IF()->drawIndexedPrimitiveUP(
						VUGFX_PT_TRIANGLELIST,				// PrimitiveType
						0,									// MinVertexIndex
						pData->mpAsset->getVertCount(),		// NumVertices
						material.mTriCount,					// PrimitiveCount
						&pData->mpAsset->getIndex(material.mStartIndex),	// IndexData
						&pData->mpAsset->getVert(0)			// VertexStreamZeroData
					);
				}
			}
		}

		VuMatrix					mModelMat;
		const VuCollisionMeshAsset	*mpAsset;
	};

	if ( VuGfxUtil::IF()->collisionShader()->getMaterial() == VUNULL )
		return;

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelMat = modelMat;
	pData->mpAsset = this;

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, VuGfxUtil::IF()->collisionShader()->getMaterial(), VUNULL, &DrawData::callback);

	if ( sDrawEdges )
		drawEdges(modelMat);
}

//*****************************************************************************
void VuCollisionMeshAsset::drawEdges(const VuMatrix &modelMat) const
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			int maxInds = VuScratchPad::SIZE/2;

			VUUINT16 *pHardInds = (VUUINT16 *)VuScratchPad::get(VuScratchPad::GRAPHICS);
			VUUINT16 *pSoftInds = (VUUINT16 *)pHardInds + maxInds/2;
			int hardIndexCount = 0;
			int softIndexCount = 0;
			for ( int iTri = 0; iTri < pData->mpAsset->getTriCount(); iTri++ )
			{
				int edgeFlags = pData->mpAsset->getTriangle(iTri).mFlags;
				if ( edgeFlags & HARD_EDGE_01 )
				{
					pHardInds[hardIndexCount++] = pData->mpAsset->getIndex(iTri*3 + 0);
					pHardInds[hardIndexCount++] = pData->mpAsset->getIndex(iTri*3 + 1);
				}
				else
				{
					pSoftInds[softIndexCount++] = pData->mpAsset->getIndex(iTri*3 + 0);
					pSoftInds[softIndexCount++] = pData->mpAsset->getIndex(iTri*3 + 1);
				}

				if ( edgeFlags & HARD_EDGE_12 )
				{
					pHardInds[hardIndexCount++] = pData->mpAsset->getIndex(iTri*3 + 1);
					pHardInds[hardIndexCount++] = pData->mpAsset->getIndex(iTri*3 + 2);
				}
				else
				{
					pSoftInds[softIndexCount++] = pData->mpAsset->getIndex(iTri*3 + 1);
					pSoftInds[softIndexCount++] = pData->mpAsset->getIndex(iTri*3 + 2);
				}

				if ( edgeFlags & HARD_EDGE_20 )
				{
					pHardInds[hardIndexCount++] = pData->mpAsset->getIndex(iTri*3 + 2);
					pHardInds[hardIndexCount++] = pData->mpAsset->getIndex(iTri*3 + 0);
				}
				else
				{
					pSoftInds[softIndexCount++] = pData->mpAsset->getIndex(iTri*3 + 2);
					pSoftInds[softIndexCount++] = pData->mpAsset->getIndex(iTri*3 + 0);
				}
			}

			VuGfx::IF()->setDepthStencilState(VuGfxUtil::IF()->getLessEqualDepthStencilState());

			if ( hardIndexCount )
			{

				VuGfxUtil::IF()->collisionShader()->setConstants(pData->mModelMat, VuColor(255,0,255));
				VuGfx::IF()->drawIndexedPrimitiveUP(
					VUGFX_PT_LINELIST,					// PrimitiveType
					0,									// MinVertexIndex
					pData->mpAsset->getVertCount(),		// NumVertices
					hardIndexCount/2,					// PrimitiveCount
					pHardInds,							// IndexData
					&pData->mpAsset->getVert(0)			// VertexStreamZeroData
				);
			}

			if ( softIndexCount )
			{
				VuGfxUtil::IF()->collisionShader()->setConstants(pData->mModelMat, VuColor(0,255,255));
				VuGfx::IF()->drawIndexedPrimitiveUP(
					VUGFX_PT_LINELIST,					// PrimitiveType
					0,									// MinVertexIndex
					pData->mpAsset->getVertCount(),		// NumVertices
					softIndexCount/2,					// PrimitiveCount
					pSoftInds,							// IndexData
					&pData->mpAsset->getVert(0)			// VertexStreamZeroData
				);
			}

			// restore default render state
			VuGfx::IF()->setDepthStencilState(VuGfxUtil::IF()->getDefaultDepthStencilState());
		}

		VuMatrix					mModelMat;
		const VuCollisionMeshAsset	*mpAsset;
	};

	if ( VuGfxUtil::IF()->collisionShader()->getMaterial() == VUNULL )
		return;

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelMat = modelMat;
	pData->mpAsset = this;

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, VuGfxUtil::IF()->collisionShader()->getMaterial(), VUNULL, &DrawData::callback);
}

//*****************************************************************************
void VuCollisionMeshAsset::adjustInternalEdgeContacts(btManifoldPoint &cp, const btCollisionObject *colObj, int triIndex)
{
	const VuTriangle &tri = getTriangle(triIndex);

	const VuVector3 &v0 = mVerts[mIndices[triIndex*3 + 0]];
	const VuVector3 &v1 = mVerts[mIndices[triIndex*3 + 1]];
	const VuVector3 &v2 = mVerts[mIndices[triIndex*3 + 2]];

	VuVector3 contact = VuDynamicsUtil::toVuVector3(cp.m_localPointB);

	bool hardEdgeCollision = false;

	// test edge 0-1
	if ( (tri.mFlags & HARD_EDGE_01) && VuMathUtil::distPointLineSeg(contact, v0, v1) < EDGE_COLLISION_DISTANCE )
		hardEdgeCollision = true;

	// test edge 1-2
	if ( (tri.mFlags & HARD_EDGE_12) && VuMathUtil::distPointLineSeg(contact, v1, v2) < EDGE_COLLISION_DISTANCE )
		hardEdgeCollision = true;

	// test edge 2-0
	if ( (tri.mFlags & HARD_EDGE_20) && VuMathUtil::distPointLineSeg(contact, v2, v0) < EDGE_COLLISION_DISTANCE )
		hardEdgeCollision = true;

	if ( !hardEdgeCollision )
	{
		VuVector3 triNormal = VuCross(v1 - v0, v2 - v0).normal();
		cp.m_normalWorldOnB = colObj->getWorldTransform().getBasis()*VuDynamicsUtil::toBtVector3(triNormal);
	}
}

//*****************************************************************************
void VuCollisionMeshAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Models");

	VuAssetUtil::addFileProperty(schema, "File", "json");
	VuAssetUtil::addBoolProperty(schema, "FlipX", false, "Flip mesh on X-Axis");

	VuJsonContainer data;
	VuJsonReader reader;
	if ( reader.loadFromFile(data, VuFile::IF()->getRootPath() + creationInfo["File"].asString()) )
	{
		const VuJsonContainer &scene = data["VuGfxScene"];

		std::set<std::string> sceneMaterials;
		VuGfxSceneUtil::gatherSceneMaterialNames(scene, sceneMaterials);

		for ( std::set<std::string>::const_iterator iter = sceneMaterials.begin(); iter != sceneMaterials.end(); iter++ )
			VuAssetUtil::addAssetProperty(schema, *iter, "VuCollisionMaterialAsset", "None");
	}
}

//*****************************************************************************
bool VuCollisionMeshAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();
	bool flipX = creationInfo["FlipX"].asBool();

	// load json document
	VuJsonContainer doc;
	VuJsonReader reader;
	if ( !reader.loadFromFile(doc, VuFile::IF()->getRootPath() + fileName) )
		return VUWARNING("Unable to load collision mesh asset %s: %s", fileName.c_str(), reader.getLastError().c_str());

	const VuJsonContainer &assetData = doc["VuGfxScene"];

	Verts verts;
	Indices indices;
	Triangles triangles;
	Materials materials;
	VuAabb aabb;
	VuOptimizedBvh *pBvh = VUNULL;

	// build tri mesh
	{
		VuGfxSceneTriMeshBuilder builder(bakeParams.mPlatform, bakeParams.mSku, bakeParams.mLanguage, creationInfo, assetData);
		builder.build(flipX);

		// dependencies
		for ( int i = 0; i < builder.mMaterials.size(); i++ )
			bakeParams.mDependencies.addAsset("VuCollisionMaterialAsset", builder.mMaterials[i].mAssetName);

		// copy verts
		verts.resize(builder.mVerts.size());
		for ( int i = 0; i < builder.mVerts.size(); i++ )
			verts[i] = builder.mVerts[i];

		// copy indices
		indices.resize(builder.mIndices.size());
		for ( int i = 0; i < builder.mIndices.size(); i++ )
			indices[i] = (VUUINT16)builder.mIndices[i];

		// allocate triangles
		int triCount = builder.mTris.size();
		triangles.resize(triCount);
		memset(&triangles.begin(), 0, triCount*sizeof(triangles[0]));

		// copy triangle info
		for ( int i = 0; i < triCount; i++ )
		{
			const VuGfxSceneTriMeshBuilder::Triangle &tri = builder.mTris[i];

			if ( tri.mMaterialIndex > 0x1f )
				return VUWARNING("Exceeded 15 collision materials in: %s", fileName.c_str());

			triangles[i].mFlags = (VUUINT8)tri.mMaterialIndex;
		}

		// handle materials
		int materialCount = (int)builder.mMaterials.size();
		materials.resize(materialCount);
		memset(&materials[0], 0, sizeof(materials[0])*materialCount);
		for ( int i = 0; i < materialCount; i++ )
		{
			const VuGfxSceneTriMeshBuilder::Material &material = builder.mMaterials[i];

			// hard edge threshold
			materials[i].mHardEdgeThreshold = VuCos(VuDegreesToRadians(material.mHardEdgeThreshold));

			// surface type
			{
				VU_STRNCPY(materials[i].mSurfaceTypeName, sizeof(materials[i].mSurfaceTypeName), material.mSurfaceType.c_str(), sizeof(materials[i].mSurfaceTypeName));
				materials[i].mSurfaceTypeName[sizeof(materials[i].mSurfaceTypeName) - 1] = '\0';
				materials[i].mSurfaceTypeID = VuDynamics::IF() ? VuDynamics::IF()->getSurfaceTypeID(materials[i].mSurfaceTypeName) : 0;
			}

			// corona collision
			if ( material.mCoronaCollision )
				materials[i].mFlags |= VuMaterial::IS_CORONA_COLLISION;

			// receive shadows
			if ( material.mReceiveShadows )
				materials[i].mFlags |= VuMaterial::RECEIVE_SHADOWS;

			// ignore baked shadows
			if ( material.mIgnoreBakedShadows )
				materials[i].mFlags |= VuMaterial::IGNORE_BAKED_SHADOW;

			// index data
			{
				materials[i].mStartIndex = 0xffff;
				materials[i].mTriCount = 0;
				for ( int iTri = 0; iTri < triCount; iTri++ )
				{
					if ( triangles[iTri].getMaterialIndex() == i )
					{
						materials[i].mFlags |= VuMaterial::IS_USED;
						materials[i].mStartIndex = (VUUINT16)VuMin(materials[i].mStartIndex, iTri*3);
						materials[i].mTriCount++;
					}
				}
			}
		}

		// handle triangle edges
		{
			const int *indices = &builder.mIndices[0];
			const VuVector3 *verts = &builder.mVerts[0];
			for ( int iTriA = 0; iTriA < triCount; iTriA++ )
			{
				VUUINT16 ia0 = (VUUINT16)indices[iTriA*3 + 0];
				VUUINT16 ia1 = (VUUINT16)indices[iTriA*3 + 1];
				VUUINT16 ia2 = (VUUINT16)indices[iTriA*3 + 2];
				VUUINT32 ea01 = ia0 << 16 | ia1;
				VUUINT32 ea12 = ia1 << 16 | ia2;
				VUUINT32 ea20 = ia2 << 16 | ia0;
				float hardEdgeThreshold = materials[triangles[iTriA].getMaterialIndex()].mHardEdgeThreshold;

				for ( int iTriB = 0; iTriB < triCount; iTriB++ )
				{
					VUUINT16 ib0 = (VUUINT16)indices[iTriB*3 + 0];
					VUUINT16 ib1 = (VUUINT16)indices[iTriB*3 + 1];
					VUUINT16 ib2 = (VUUINT16)indices[iTriB*3 + 2];
					VUUINT32 eb10 = ib1 << 16 | ib0;
					VUUINT32 eb21 = ib2 << 16 | ib1;
					VUUINT32 eb02 = ib0 << 16 | ib2;

					// test edge 0-1
					if ( ea01 == eb10 || ea01 == eb21 || ea01 == eb02)
						if ( isHardEdge(iTriA, iTriB, ia0, ia1, indices, verts, hardEdgeThreshold) )
							triangles[iTriA].mFlags |= HARD_EDGE_01;

					// test edge 1-2
					if ( ea12 == eb10 || ea12 == eb21 || ea12 == eb02)
						if ( isHardEdge(iTriA, iTriB, ia1, ia2, indices, verts, hardEdgeThreshold) )
							triangles[iTriA].mFlags |= HARD_EDGE_12;

					// test edge 2-0
					if ( ea20 == eb10 || ea20 == eb21 || ea20 == eb02 )
						if ( isHardEdge(iTriA, iTriB, ia2, ia0, indices, verts, hardEdgeThreshold) )
							triangles[iTriA].mFlags |= HARD_EDGE_20;
				}
			}
		}
	}

	if ( verts.size() == 0 || indices.size() == 0 )
		return VUWARNING("No collision geometry found in: %s", fileName.c_str());

	// calculate aabb
	for ( int i = 0; i < verts.size(); i++ )
		aabb.addPoint(verts[i]);

	// make sure there are no degenerate triangles
	int triCount = indices.size()/3;
	for ( int i = 0; i < triCount; i++ )
	{
		VuVector3 &v0 = verts[indices[i*3 + 0]];
		VuVector3 &v1 = verts[indices[i*3 + 1]];
		VuVector3 &v2 = verts[indices[i*3 + 2]];

		float area = 0.5f*VuCross(v1 - v0, v2 - v0).mag();
		if ( area < FLT_EPSILON )
			return VUWARNING("Degenerate collision triangle in: %s", fileName.c_str());
	}

	VuSimpleStridingMesh stridingMesh(verts, indices);

	// build bhv
	{
		void *mem = btAlignedAlloc(sizeof(btOptimizedBvh), 16);
		pBvh = new (mem) VuOptimizedBvh;
		pBvh->build(&stridingMesh, true, VuDynamicsUtil::toBtVector3(aabb.mMin), VuDynamicsUtil::toBtVector3(aabb.mMax));
	}

	// arrays
	writer.writeArray(verts);
	writer.writeArray(indices);
	serialize(writer, triangles);
	serialize(writer, materials);

	// triangle info map
	pBvh->serialize(writer);

	// misc
	writer.writeValue(aabb);

	// clean up
	pBvh->~VuOptimizedBvh();
	btAlignedFree(pBvh);

	return true;
}

//*****************************************************************************
bool VuCollisionMeshAsset::load(VuBinaryDataReader &reader)
{
	// arrays
	reader.readArray(mVerts);
	reader.readArray(mIndices);
	reader.readArray(mTriangles);
	reader.readArray(mMaterials);

	// bvh
	{
		void *mem = btAlignedAlloc(sizeof(btOptimizedBvh), 16);
		mpBvh = new (mem) VuOptimizedBvh;
		mpBvh->deserialize(reader);
	}

	// misc
	reader.readValue(mAabb);

	// surface type index may have changed
	for ( int i = 0; i < mMaterials.size(); i++ )
		mMaterials[i].mSurfaceTypeID = VuDynamics::IF() ? VuDynamics::IF()->getSurfaceTypeID(mMaterials[i].mSurfaceTypeName) : 0;

	return true;
}

//*****************************************************************************
void VuCollisionMeshAsset::unload()
{
	mVerts.clear();
	mIndices.clear();
	mTriangles.clear();
	mMaterials.clear();

	if ( mpBvh )
	{
		mpBvh->~VuOptimizedBvh();
		btAlignedFree(mpBvh);
		mpBvh = VUNULL;
	}
}

//*****************************************************************************
void VuCollisionMeshAsset::serialize(VuBinaryDataWriter &writer, const Triangles &triangles)
{
	int count = triangles.size();
	writer.writeValue(count);
	for ( int i = 0; i < count; i++ )
	{
		const VuTriangle &tri = triangles[i];

		writer.writeValue(tri.mFlags);
	}
}

//*****************************************************************************
void VuCollisionMeshAsset::serialize(VuBinaryDataWriter &writer, const Materials &materials)
{
	int count = materials.size();
	writer.writeValue(count);
	for ( int i = 0; i < count; i++ )
	{
		const VuMaterial &mat = materials[i];

		writer.writeData(mat.mSurfaceTypeName, sizeof(mat.mSurfaceTypeName));
		writer.writeValue(mat.mHardEdgeThreshold);
		writer.writeValue(mat.mStartIndex);
		writer.writeValue(mat.mTriCount);
		writer.writeValue(mat.mSurfaceTypeID);
		writer.writeValue(mat.mFlags);
		writer.writeValue(mat.mPad1);
		writer.writeValue(mat.mPad2);
	}
}

//*****************************************************************************
bool VuCollisionMeshAsset::isHardEdge(int iTriA, int iTriB, int e0, int e1, const int *indices, const VuVector3 *verts, float threshold)
{
	VuVector3 va0 = verts[indices[iTriA*3 + 0]];
	VuVector3 va1 = verts[indices[iTriA*3 + 1]];
	VuVector3 va2 = verts[indices[iTriA*3 + 2]];
	VuVector3 norA = VuCross(va1 - va0, va2 - va0).normal();
	VuVector4 planeA = VuMathUtil::planeFromNormalPoint(norA, va0);

	VuVector3 vb0 = verts[indices[iTriB*3 + 0]];
	VuVector3 vb1 = verts[indices[iTriB*3 + 1]];
	VuVector3 vb2 = verts[indices[iTriB*3 + 2]];
	VuVector3 norB = VuCross(vb1 - vb0, vb2 - vb0).normal();
	VuVector3 cenB = (vb0 + vb1 + vb2)/3.0f;

//	VuVector3 ve0 = verts[indices[e0]];
//	VuVector3 ve1 = verts[indices[e1]];

	// external edge?
	if ( VuMathUtil::distPointPlane(cenB, planeA) < 0.0f )
	{
		// check threshold
		if ( VuDot(norA, norB) < threshold )
			return true;
	}

	return false;
}
