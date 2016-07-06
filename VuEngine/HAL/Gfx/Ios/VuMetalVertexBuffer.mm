//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the vertex buffer interface class.
//
//*****************************************************************************

#include "VuMetalVertexBuffer.h"
#include "VuMetalGfx.h"


//*****************************************************************************
VuMetalVertexBuffer::VuMetalVertexBuffer(int size):
	VuVertexBuffer(size)
{
}

//*****************************************************************************
VuMetalVertexBuffer::~VuMetalVertexBuffer()
{
}

//*****************************************************************************
void VuMetalVertexBuffer::resize(int newSize)
{
	mMTLBuffer = [VuMetalGfx::getDevice() newBufferWithLength:newSize options:MTLResourceOptionCPUCacheModeDefault];

	// update size
	mSize = newSize;
}

//*****************************************************************************
void VuMetalVertexBuffer::setData(const void *pData, int size)
{
	memcpy([mMTLBuffer contents], pData, size);
}

//*****************************************************************************
VuMetalVertexBuffer *VuMetalVertexBuffer::create(int size, VUUINT32 usageFlags)
{
	// allocate buffer
	VuMetalVertexBuffer *pVertexBuffer = new VuMetalVertexBuffer(size);
	
	pVertexBuffer->mMTLBuffer = [VuMetalGfx::getDevice() newBufferWithLength:size options:MTLResourceOptionCPUCacheModeDefault];
	
	return pVertexBuffer;
}
