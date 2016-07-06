//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 implementation of the index buffer interface class.
//
//*****************************************************************************

#include <gnm.h>
#include "VuPs4IndexBuffer.h"
#include "VuPs4Gfx.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"

using namespace sce;


//*****************************************************************************
VuPs4IndexBuffer::VuPs4IndexBuffer(int count):
	VuIndexBuffer(count),
	mpBuffer(VUNULL)
{
}

//*****************************************************************************
VuPs4IndexBuffer::~VuPs4IndexBuffer()
{
	VuPs4Gfx::IF()->garlicAllocator().release(mpBuffer);
}

//*****************************************************************************
void VuPs4IndexBuffer::resize(int newCount)
{
	VuPs4Gfx::IF()->garlicAllocator().release(mpBuffer);
	mpBuffer = VuPs4Gfx::IF()->garlicAllocator().allocate(newCount*2, Gnm::kAlignmentOfBufferInBytes);

	// update size
	mCount = newCount;
}

//*****************************************************************************
void VuPs4IndexBuffer::setData(const VUUINT16 *pData, int count)
{
	VUASSERT(count <= mCount, "VuPs4IndexBuffer::setData() bounds exceeded");
	memcpy(mpBuffer, pData, count*2);
}

//*****************************************************************************
VuPs4IndexBuffer *VuPs4IndexBuffer::create(int count, VUUINT32 usageFlags)
{
	// allocate buffer
	VuPs4IndexBuffer *pIndexBuffer = new VuPs4IndexBuffer(count);

	pIndexBuffer->mpBuffer = VuPs4Gfx::IF()->garlicAllocator().allocate(count*2, Gnm::kAlignmentOfBufferInBytes);

	return pIndexBuffer;
}