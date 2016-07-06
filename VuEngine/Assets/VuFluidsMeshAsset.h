//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Fluids Mesh Asset class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Assets/VuAsset.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuAabb.h"
#include "VuEngine/Containers/VuArray.h"

class VuJsonContainer;
class VuColor;


class VuFluidsMeshAsset : public VuAsset
{
	DECLARE_RTTI

protected:
	~VuFluidsMeshAsset() {}
public:
	struct Edge
	{
		int		vi0;
		int		vi1;

		void	serialize(VuBinaryDataWriter &writer);
	};
	struct Tri
	{
		int			vi0;
		int			vi1;
		int			vi2;
		int			ei0;
		int			ei1;
		int			ei2;
		float		mArea;
		VuVector3	mNormal;
		VuVector3	mCentroid;

		void		serialize(VuBinaryDataWriter &writer);
	};

	VuFluidsMeshAsset();

	int					getVertCount() const	{ return (int)mVerts.size(); }
	int					getEdgeCount() const	{ return (int)mEdges.size(); }
	int					getTriCount() const		{ return (int)mTris.size(); }
	const VuVector3		*getVerts() const		{ return &mVerts[0]; }
	const Edge			*getEdges() const		{ return &mEdges[0]; }
	const Tri			*getTris() const		{ return &mTris[0]; }
	float				getTotalArea() const	{ return mTotalArea; }
	float				getTotalVolume() const	{ return mTotalVolume; }
	const VuAabb		&getAabb() const		{ return mAabb; }

	void				draw(const VuColor &color, const VuMatrix &modelMat) const;

	static void			schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
	static bool			bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams);

private:
	virtual bool		load(VuBinaryDataReader &reader);
	virtual void		unload();

	typedef VuArray<VuVector3> Verts;
	typedef VuArray<Edge> Edges;
	typedef VuArray<Tri> Tris;

	static int			addEdge(Edges &edges, int vi0, int vi1);
	static bool			verifyClosedMesh(const Tris &tris);
	static float		calculateTotalVolume(const Tris &tris, const Verts &verts, const VuAabb &aabb);

	VuAabb	mAabb;
	Verts	mVerts;
	Edges	mEdges;
	Tris	mTris;
	float	mTotalArea;
	float	mTotalVolume;
};
