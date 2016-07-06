//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  String Mesh class
// 
//*****************************************************************************

#include "VuStridingMesh.h"
#include "VuEngine/Assets/VuCollisionMeshAsset.h"


//*****************************************************************************
void VuStridingMesh::getLockedReadOnlyVertexIndexBase(const unsigned char **vertexbase, int &numverts, PHY_ScalarType &type, int &stride, const unsigned char **indexbase, int &indexstride, int &numfaces, PHY_ScalarType &indicestype, int subpart) const
{
	VUASSERT(subpart == 0, "VuCollisionMeshAssetImpl::getLockedReadOnlyVertexIndexBase() invalid sub-part");

	*vertexbase = (const unsigned char *)&mpCollisionMeshAsset->getVert(0);
	numverts = mpCollisionMeshAsset->getVertCount();
	type = PHY_FLOAT;
	stride = sizeof(mpCollisionMeshAsset->getVert(0));

	*indexbase =  (const unsigned char *)&mpCollisionMeshAsset->getIndex(0);
	numfaces = mpCollisionMeshAsset->getIndexCount()/3;
	indicestype = PHY_SHORT;
	indexstride = 3*sizeof(mpCollisionMeshAsset->getIndex(0));
}

//*****************************************************************************
int VuStridingMesh::getNumSubParts() const	
{
	return mpCollisionMeshAsset->getIndexCount() ? 1 : 0;
}

//*****************************************************************************
void VuSimpleStridingMesh::getLockedReadOnlyVertexIndexBase(const unsigned char **vertexbase, int &numverts, PHY_ScalarType &type, int &stride, const unsigned char **indexbase, int &indexstride, int &numfaces, PHY_ScalarType &indicestype, int subpart) const
{
	VUASSERT(subpart == 0, "VuCollisionMeshAssetImpl::getLockedReadOnlyVertexIndexBase() invalid sub-part");

	*vertexbase = (const unsigned char *)&mVerts[0];
	numverts = mVerts.size();
	type = PHY_FLOAT;
	stride = sizeof(mVerts[0]);

	*indexbase =  (const unsigned char *)&mIndices[0];
	numfaces = mIndices.size()/3;
	indicestype = PHY_SHORT;
	indexstride = 3*sizeof(mIndices[0]);
}

//*****************************************************************************
int VuSimpleStridingMesh::getNumSubParts() const	
{
	return mIndices.size() ? 1 : 0;
}
