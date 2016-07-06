//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 implementation of the vertex buffer interface class.
//
//*****************************************************************************

#include <gnm.h>
#include "VuPs4VertexBuffer.h"
#include "VuPs4Gfx.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"

using namespace sce;


//*****************************************************************************
VuPs4VertexBuffer::VuPs4VertexBuffer(int size):
	VuVertexBuffer(size),
	mpBuffer(VUNULL)
{
}

//*****************************************************************************
VuPs4VertexBuffer::~VuPs4VertexBuffer()
{
	VuPs4Gfx::IF()->garlicAllocator().release(mpBuffer);
}

//*****************************************************************************
void VuPs4VertexBuffer::resize(int newSize)
{
	VuPs4Gfx::IF()->garlicAllocator().release(mpBuffer);
	mpBuffer = VuPs4Gfx::IF()->garlicAllocator().allocate(newSize, Gnm::kAlignmentOfBufferInBytes);

	// update size
	mSize = newSize;
}

//*****************************************************************************
void VuPs4VertexBuffer::setData(const void *pData, int size)
{
	VUASSERT(size <= mSize, "VuPs4VertexBuffer::setData() bounds exceeded");
	memcpy(mpBuffer, pData, size);
}

//*****************************************************************************
VuPs4VertexBuffer *VuPs4VertexBuffer::create(int size, VUUINT32 usageFlags)
{
	// allocate buffer
	VuPs4VertexBuffer *pVertexBuffer = new VuPs4VertexBuffer(size);

	pVertexBuffer->mpBuffer = VuPs4Gfx::IF()->garlicAllocator().allocate(size, Gnm::kAlignmentOfBufferInBytes);

	return pVertexBuffer;
}
