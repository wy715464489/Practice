//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the index buffer interface class.
//
//*****************************************************************************

#include "VuMetalIndexBuffer.h"
#include "VuMetalGfx.h"


//*****************************************************************************
VuMetalIndexBuffer::VuMetalIndexBuffer(int count):
	VuIndexBuffer(count)
{
}

//*****************************************************************************
VuMetalIndexBuffer::~VuMetalIndexBuffer()
{
}

//*****************************************************************************
void VuMetalIndexBuffer::resize(int newCount)
{
	mMTLBuffer = [VuMetalGfx::getDevice() newBufferWithLength:newCount*2 options:MTLResourceOptionCPUCacheModeDefault];

	// update size
	mCount = newCount;
}

//*****************************************************************************
void VuMetalIndexBuffer::setData(const VUUINT16 *pData, int count)
{
	memcpy([mMTLBuffer contents], pData, count*2);
}

//*****************************************************************************
VuMetalIndexBuffer *VuMetalIndexBuffer::create(int count, VUUINT32 usageFlags)
{
	// allocate buffer
	VuMetalIndexBuffer *pIndexBuffer = new VuMetalIndexBuffer(count);
	
	pIndexBuffer->mMTLBuffer = [VuMetalGfx::getDevice() newBufferWithLength:count*2 options:MTLResourceOptionCPUCacheModeDefault];

	return pIndexBuffer;
}
