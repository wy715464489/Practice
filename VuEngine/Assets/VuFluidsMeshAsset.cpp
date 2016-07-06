//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Fluids Mesh Asset class
// 
//*****************************************************************************

#include <float.h>
#include "VuFluidsMeshAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxScene/VuGfxSceneTriMeshBuilder.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/Shaders/VuCollisionShader.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Memory/VuScratchPad.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


IMPLEMENT_RTTI(VuFluidsMeshAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuFluidsMeshAsset);


//*****************************************************************************
VuFluidsMeshAsset::VuFluidsMeshAsset():
	mTotalArea(0),
	mTotalVolume(0)
{
}

//*****************************************************************************
void VuFluidsMeshAsset::draw(const VuColor &color, const VuMatrix &modelMat) const
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			// put indices onto scratch pad
			VUUINT16 *pScratchPad = static_cast<VUUINT16 *>(VuScratchPad::get(VuScratchPad::GRAPHICS));
			VUUINT16 *pIndex = pScratchPad;
			const Tri *pTri = pData->mpAsset->getTris();
			for ( int i = 0; i < pData->mpAsset->getTriCount(); i++ )
			{
				*pIndex++ = (VUUINT16)pTri->vi0;
				*pIndex++ = (VUUINT16)pTri->vi1;
				*pIndex++ = (VUUINT16)pTri->vi2;
				pTri++;
			}

			VuGfxUtil::IF()->collisionShader()->setConstants(pData->mModelMat, pData->mColor);
			VuGfx::IF()->drawIndexedPrimitiveUP(
				VUGFX_PT_TRIANGLELIST,			// PrimitiveType
				0,								// MinVertexIndex
				pData->mpAsset->getVertCount(),	// NumVertices
				pData->mpAsset->getTriCount(),	// PrimitiveCount
				pScratchPad,					// IndexData
				pData->mpAsset->getVerts()		// VertexStreamZeroData
			);
		}

		VuMatrix				mModelMat;
		VuColor					mColor;
		const VuFluidsMeshAsset	*mpAsset;
	};

	if ( VuGfxUtil::IF()->collisionShader()->getMaterial() == VUNULL )
		return;

	DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
	pData->mModelMat = modelMat;
	pData->mColor = color;
	pData->mpAsset = this;

	VuGfxSort::IF()->submitDrawCommand<false>(VuGfxSort::TRANS_OPAQUE, VuGfxUtil::IF()->collisionShader()->getMaterial(), VUNULL, &DrawData::callback);
}

//*****************************************************************************
void VuFluidsMeshAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Models");

	VuAssetUtil::addFileProperty(schema, "File", "json");
}

//*****************************************************************************
bool VuFluidsMeshAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();

	// load json document
	VuJsonContainer doc;
	VuJsonReader reader;
	if ( !reader.loadFromFile(doc, VuFile::IF()->getRootPath() + fileName) )
		return VUWARNING("Unable to load fluids mesh asset %s: %s", fileName.c_str(), reader.getLastError().c_str());

	const VuJsonContainer &assetData = doc["VuGfxScene"];

	VuAabb aabb;
	Verts verts;
	Edges edges;
	Tris tris;
	float totalArea = 0;
	float totalVolume = 0;

	// build tri mesh
	{
		VuGfxSceneTriMeshBuilder builder(bakeParams.mPlatform, bakeParams.mSku, bakeParams.mLanguage, creationInfo, assetData);
		builder.build(false);

		// copy verts
		int vertCount = builder.mVerts.size();
		verts.resize(vertCount);
		for ( int i = 0; i < vertCount; i++ )
			verts[i] = builder.mVerts[i];

		// generate triangles and edges
		int triCount = builder.mIndices.size()/3;
		tris.resize(triCount);
		for ( int i = 0; i < triCount; i++ )
		{
			// copy indices
			int vi0 = builder.mIndices[i*3 + 0];
			int vi1 = builder.mIndices[i*3 + 1];
			int vi2 = builder.mIndices[i*3 + 2];
			tris[i].vi0 = vi0;
			tris[i].vi1 = vi1;
			tris[i].vi2 = vi2;

			// build edges
			tris[i].ei0 = addEdge(edges, vi0, vi1);
			tris[i].ei1 = addEdge(edges, vi1, vi2);
			tris[i].ei2 = addEdge(edges, vi2, vi0);

			// calculate area & normal
			const VuVector3 &v0 = verts[vi0];
			const VuVector3 &v1 = verts[vi1];
			const VuVector3 &v2 = verts[vi2];
			VuVector3 normal = VuCross(v1 - v0, v2 - v0);
			tris[i].mArea = 0.5f*normal.mag();
			if ( tris[i].mArea < FLT_EPSILON )
				return VUWARNING("Degenerate fluids mesh triangle!");
			tris[i].mNormal = normal.normal();
			tris[i].mCentroid = (v0 + v1 + v2)/3.0f;
		}

		// calculate total area
		for ( int i = 0; i < triCount; i++ )
			totalArea += tris[i].mArea;

		// calculate aabb
		for ( int i = 0; i < vertCount; i++ )
			aabb.addPoint(builder.mVerts[i]);

		// make sure mesh is closed
		if ( !verifyClosedMesh(tris) )
			return VUWARNING("Fluids mesh asset %s is not closed!", fileName.c_str());

		// calculate total volume
		totalVolume = calculateTotalVolume(tris, verts, aabb);
	}

	// arrays
	writer.writeArray(verts);

	int count = edges.size();
	writer.writeValue(count);
	for ( int i = 0; i < count; i++ )
		edges[i].serialize(writer);

	count = tris.size();
	writer.writeValue(count);
	for ( int i = 0; i < count; i++ )
		tris[i].serialize(writer);

	// misc
	writer.writeValue(aabb);
	writer.writeValue(totalArea);
	writer.writeValue(totalVolume);

	return true;
}

//*****************************************************************************
bool VuFluidsMeshAsset::load(VuBinaryDataReader &reader)
{
	// arrays
	reader.readArray(mVerts);
	reader.readArray(mEdges);
	reader.readArray(mTris);

	// misc
	reader.readValue(mAabb);
	reader.readValue(mTotalArea);
	reader.readValue(mTotalVolume);

	return true;
}

//*****************************************************************************
void VuFluidsMeshAsset::unload()
{
	mAabb.reset();
	mVerts.clear();
	mEdges.clear();
	mTris.clear();
	mTotalArea = 0;
	mTotalVolume = 0;
}

//*****************************************************************************
int VuFluidsMeshAsset::addEdge(Edges &edges, int vi0, int vi1)
{
	for ( int i = 0; i < (int)edges.size(); i++ )
		if ( (vi0 == edges[i].vi0 && vi1 == edges[i].vi1) || (vi1 == edges[i].vi0 && vi0 == edges[i].vi1) )
			return i;

	Edge edge;
	edge.vi0 = vi0;
	edge.vi1 = vi1;
	edges.push_back(edge);

	return (int)edges.size() - 1;
}

//*****************************************************************************
bool VuFluidsMeshAsset::verifyClosedMesh(const Tris &tris)
{
	for ( int i = 0; i < tris.size(); i++ )
	{
		const Tri &tri1 = tris[i];

		// find number of tris with matching edges
		int count = 0;
		for ( int j = 0; j < tris.size(); j++ )
		{
			const Tri &tri2 = tris[j];
			if ( i != j )
			{
				if ( tri1.ei0 == tri2.ei0 || tri1.ei0 == tri2.ei1 || tri1.ei0 == tri2.ei2 ||
					 tri1.ei1 == tri2.ei0 || tri1.ei1 == tri2.ei1 || tri1.ei1 == tri2.ei2 ||
					 tri1.ei2 == tri2.ei0 || tri1.ei2 == tri2.ei1 || tri1.ei2 == tri2.ei2 )
				{
					count++;
				}
			}
		}

		// must be exactly three tris with matching edges
		if ( count != 3 )
			return false;
	}

	return true;
}

//*****************************************************************************
float VuFluidsMeshAsset::calculateTotalVolume(const Tris &tris, const Verts &verts, const VuAabb &aabb)
{
	float fTotalVolume = 0.0f;

	float waterSurfaceZ = aabb.mMax.mZ;
	for ( int i = 0; i < tris.size(); i++ )
	{
		const Tri &tri = tris[i];

		// calculate volume and centroid of water mass above triangle
		// (volume consists of 3 tetrahedra defined by verts 0, 1, 2,
		//  and the verts on the water surface directly above them)
		const VuVector3 &A = verts[tri.vi0];
		const VuVector3 &B = verts[tri.vi1];
		const VuVector3 &C = verts[tri.vi2];
		VuVector3 vE0 = B - A;
		VuVector3 vE1 = C - B;
		VuVector3 vE2 = A - C;
		float h0 = waterSurfaceZ - A.mZ;
		float h1 = waterSurfaceZ - B.mZ;
		float h2 = waterSurfaceZ - C.mZ;

		float fVolume1 = h0*VuAbs((vE0.mX)*(vE2.mY) - (vE2.mX)*(vE0.mY))/6.0f;
		float fVolume2 = h1*VuAbs((vE1.mX)*(vE0.mY) - (vE0.mX)*(vE1.mY))/6.0f;
		float fVolume3 = h2*VuAbs((vE2.mX)*(vE1.mY) - (vE1.mX)*(vE2.mY))/6.0f;
		float fVolume = (fVolume1 + fVolume2 + fVolume3);

		// if tri is facing down, negate
		fVolume *= VuSelect(tri.mNormal.mZ, -1.0f, 1.0f);

		fTotalVolume += fVolume;
	}

	return fTotalVolume;
}

//*****************************************************************************
void VuFluidsMeshAsset::Edge::serialize(VuBinaryDataWriter &writer)
{
	writer.writeValue(vi0);
	writer.writeValue(vi1);
}

//*****************************************************************************
void VuFluidsMeshAsset::Tri::serialize(VuBinaryDataWriter &writer)
{
	writer.writeValue(vi0);
	writer.writeValue(vi1);
	writer.writeValue(vi2);
	writer.writeValue(ei0);
	writer.writeValue(ei1);
	writer.writeValue(ei2);
	writer.writeValue(mArea);
	writer.writeValue(mNormal);
	writer.writeValue(mCentroid);
}
