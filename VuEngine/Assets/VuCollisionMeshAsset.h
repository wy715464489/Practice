//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Collision Mesh Asset class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Assets/VuAsset.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Util/VuColor.h"

class VuColor;
class VuMatrix;
class VuOptimizedBvh;
class btOptimizedBvh;
class btManifoldPoint;
class btCollisionObject;


class VuCollisionMeshAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuCollisionMeshAsset() { unload(); }
public:
	VuCollisionMeshAsset();

	enum eEdgeFlags
	{
		// bits 0-4 reserved for material index
		HARD_EDGE_01 = 1<<5,
		HARD_EDGE_12 = 1<<6,
		HARD_EDGE_20 = 1<<7 
	};

	struct VuTriangle
	{
		VUUINT8	getMaterialIndex() const { return mFlags & 0x1f; }

		VUUINT8	mFlags; // material index & edge flags
	};

	struct VuMaterial
	{
		enum
		{
			IS_USED             = 1<<0,
			IS_CORONA_COLLISION = 1<<1,
			RECEIVE_SHADOWS     = 1<<2,
			IGNORE_BAKED_SHADOW = 1<<3,
		};

		char		mSurfaceTypeName[32];
		float		mHardEdgeThreshold;
		VUUINT16	mStartIndex;
		VUUINT16	mTriCount;
		VUUINT8		mSurfaceTypeID;
		VUUINT8		mFlags;
		VUUINT8		mPad1;
		VUUINT8		mPad2;
	};

	int							getVertCount() const				{ return (int)mVerts.size(); }
	int							getIndexCount() const				{ return (int)mIndices.size(); }
	int							getTriCount() const					{ return (int)mTriangles.size(); }
	int							getMaterialCount() const			{ return (int)mMaterials.size(); }
	const VuVector3				&getVert(int index) const			{ return mVerts[index]; }
	const VUUINT16				&getIndex(int index) const			{ return mIndices[index]; }
	const VuTriangle			&getTriangle(int index) const		{ return mTriangles[index]; }
	const VuMaterial			&getMaterial(int index) const		{ return mMaterials[index]; }
	const VuMaterial			&getTriangleMaterial(int tri) const	{ return mMaterials[mTriangles[tri].getMaterialIndex()]; }
	const VuAabb				&getAabb() const					{ return mAabb; }
	btOptimizedBvh				*getBvh() const;

	void						draw(const VuColor &color, const VuMatrix &modelMat) const;
	void						drawWithColors(const VuMatrix &modelMat) const;
	void						drawEdges(const VuMatrix &modelMat) const;

	void						adjustInternalEdgeContacts(btManifoldPoint &cp, const btCollisionObject *colObj, int triIndex);

	static void					schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool					bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

private:
	typedef VuArray<VuVector3> Verts;
	typedef VuArray<VUUINT16> Indices;
	typedef VuArray<VuTriangle> Triangles;
	typedef VuArray<VuMaterial> Materials;

	virtual bool				load(VuBinaryDataReader &reader);
	virtual void				unload();

	static void					serialize(VuBinaryDataWriter &writer, const Triangles &triangles);
	static void					serialize(VuBinaryDataWriter &writer, const Materials &materials);

	static bool					isHardEdge(int iTriA, int iTriB, int e0, int e1, const int *indices, const VuVector3 *verts, float threshold);

	Verts						mVerts;
	Indices						mIndices;
	Triangles					mTriangles;
	Materials					mMaterials;
	VuAabb						mAabb;
	VuOptimizedBvh				*mpBvh;
};
