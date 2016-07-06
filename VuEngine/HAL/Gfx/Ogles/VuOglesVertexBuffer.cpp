//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpenGL ES implementation of the vertex buffer interface class.
//
//*****************************************************************************

#include "VuOglesVertexBuffer.h"
#include "VuOglesGfx.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


//*****************************************************************************
VuOglesVertexBuffer::VuOglesVertexBuffer(int size):
	VuVertexBuffer(size), mGlUsage(0)
{
	glGenBuffers(1, &mGlBuffer);
}

//*****************************************************************************
VuOglesVertexBuffer::~VuOglesVertexBuffer()
{
	if ( !VuOglesGfx::IF()->getContextDestroyed() )
		glDeleteBuffers(1, &mGlBuffer);
}

//*****************************************************************************
void VuOglesVertexBuffer::resize(int newSize)
{
	VuOglesGfx::IF()->bindVertexBuffer(mGlBuffer);

	// resize buffer
	glBufferData(GL_ARRAY_BUFFER, newSize, VUNULL, mGlUsage);

	// update size
	mSize = newSize;
}

//*****************************************************************************
void VuOglesVertexBuffer::setData(const void *pData, int size)
{
	VuOglesGfx::IF()->bindVertexBuffer(mGlBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, pData);
}

//*****************************************************************************
VuOglesVertexBuffer *VuOglesVertexBuffer::create(int size, VUUINT32 usageFlags)
{
	// allocate buffer
	VuOglesVertexBuffer *pVertexBuffer = new VuOglesVertexBuffer(size);

	// translate usage
	pVertexBuffer->mGlUsage = GL_STATIC_DRAW;
	if ( usageFlags & VUGFX_USAGE_DYNAMIC )
		pVertexBuffer->mGlUsage = GL_DYNAMIC_DRAW;

	VuOglesGfx::IF()->bindVertexBuffer(pVertexBuffer->mGlBuffer);
	glBufferData(GL_ARRAY_BUFFER, size, VUNULL, pVertexBuffer->mGlUsage);

	return pVertexBuffer;
}
