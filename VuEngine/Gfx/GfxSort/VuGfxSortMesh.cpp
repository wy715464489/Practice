//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuGfxSortMesh class
// 
//*****************************************************************************

#include "VuGfxSortMesh.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/Gfx/VuVertexBuffer.h"
#include "VuEngine/HAL/Gfx/VuIndexBuffer.h"


//*****************************************************************************
VuGfxSortMeshDesc::VuGfxSortMeshDesc():
	mpVertexBuffer(VUNULL),
	mpIndexBuffer(VUNULL)
{
}

//*****************************************************************************
bool VuGfxSortMeshDesc::operator == (const VuGfxSortMeshDesc &other) const
{
	if ( *this < other || other < *this )
		return false;

	return true;
}

//*****************************************************************************
bool VuGfxSortMeshDesc::operator < (const VuGfxSortMeshDesc &other) const
{
	if ( mpVertexBuffer != other.mpVertexBuffer )
		return mpVertexBuffer < other.mpVertexBuffer;

	if ( mpIndexBuffer != other.mpIndexBuffer )
		return mpIndexBuffer < other.mpIndexBuffer;

	return false;
}

//*****************************************************************************
VuGfxSortMesh::VuGfxSortMesh(const VuGfxSortMeshDesc &desc):
	mDesc(desc),
	mSortKey(0),
	mRefCount(1)
{
	// add member refs
	mDesc.mpVertexBuffer->addRef();
	mDesc.mpIndexBuffer->addRef();
}

//*****************************************************************************
VuGfxSortMesh::~VuGfxSortMesh()
{
	// remove member refs
	mDesc.mpVertexBuffer->removeRef();
	mDesc.mpIndexBuffer->removeRef();
}
