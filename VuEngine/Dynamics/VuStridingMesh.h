//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  String Mesh class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Dynamics/VuDynamics.h"
#include "VuEngine/Containers/VuArray.h"

class VuCollisionMeshAsset;


class VuStridingMesh : public btStridingMeshInterface
{
public:
	VuStridingMesh() : mpCollisionMeshAsset(VUNULL) {}
	VuStridingMesh(VuCollisionMeshAsset *pCollisionMeshAsset) : mpCollisionMeshAsset(pCollisionMeshAsset) {}

	void					setCollisionMeshAsset(VuCollisionMeshAsset *pCollisionMeshAsset) { mpCollisionMeshAsset = pCollisionMeshAsset; }
	VuCollisionMeshAsset	*getCollisionMeshAsset() const { return mpCollisionMeshAsset; }

private:
	virtual void			getLockedVertexIndexBase(unsigned char **vertexbase, int &numverts, PHY_ScalarType &type, int &stride, unsigned char **indexbase, int &indexstride, int &numfaces, PHY_ScalarType &indicestype, int subpart)	{ VUASSERT(0, "VuCollisionMeshAssetImpl::getLockedVertexIndexBase() invalid call"); }
	virtual void			getLockedReadOnlyVertexIndexBase(const unsigned char **vertexbase, int &numverts, PHY_ScalarType &type, int &stride, const unsigned char **indexbase, int &indexstride, int &numfaces, PHY_ScalarType &indicestype, int subpart) const;
	virtual void			unLockVertexBase(int subpart)				{}
	virtual void			unLockReadOnlyVertexBase(int subpart) const	{}
	virtual int				getNumSubParts() const;
	virtual void			preallocateVertices(int numverts)			{ VUASSERT(0, "VuCollisionMeshAssetImpl::preallocateVertices() invalid call"); }
	virtual void			preallocateIndices(int numindices)			{ VUASSERT(0, "VuCollisionMeshAssetImpl::preallocateIndices() invalid call"); }

	VuCollisionMeshAsset	*mpCollisionMeshAsset;
};


class VuSimpleStridingMesh : public btStridingMeshInterface
{
public:
	VuSimpleStridingMesh(const VuArray<VuVector3> &verts, const VuArray<VUUINT16> &indices) : mVerts(verts), mIndices(indices) {}

private:
	virtual void			getLockedVertexIndexBase(unsigned char **vertexbase, int &numverts, PHY_ScalarType &type, int &stride, unsigned char **indexbase, int &indexstride, int &numfaces, PHY_ScalarType &indicestype, int subpart)	{ VUASSERT(0, "VuCollisionMeshAssetImpl::getLockedVertexIndexBase() invalid call"); }
	virtual void			getLockedReadOnlyVertexIndexBase(const unsigned char **vertexbase, int &numverts, PHY_ScalarType &type, int &stride, const unsigned char **indexbase, int &indexstride, int &numfaces, PHY_ScalarType &indicestype, int subpart) const;
	virtual void			unLockVertexBase(int subpart)				{}
	virtual void			unLockReadOnlyVertexBase(int subpart) const	{}
	virtual int				getNumSubParts() const;
	virtual void			preallocateVertices(int numverts)			{ VUASSERT(0, "VuCollisionMeshAssetImpl::preallocateVertices() invalid call"); }
	virtual void			preallocateIndices(int numindices)			{ VUASSERT(0, "VuCollisionMeshAssetImpl::preallocateIndices() invalid call"); }

	const VuArray<VuVector3>	&mVerts;
	const VuArray<VUUINT16>		&mIndices;
};