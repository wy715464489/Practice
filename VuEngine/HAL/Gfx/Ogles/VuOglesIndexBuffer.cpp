//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpenGL ES implementation of the index buffer interface class.
//
//*****************************************************************************

#include "VuOglesIndexBuffer.h"
#include "VuOglesGfx.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


//*****************************************************************************
VuOglesIndexBuffer::VuOglesIndexBuffer(int count):
	VuIndexBuffer(count), mGlUsage(0)
{
	glGenBuffers(1, &mGlBuffer);
}

//*****************************************************************************
VuOglesIndexBuffer::~VuOglesIndexBuffer()
{
	if ( !VuOglesGfx::IF()->getContextDestroyed() )
		glDeleteBuffers(1, &mGlBuffer);
}

//*****************************************************************************
void VuOglesIndexBuffer::resize(int newCount)
{
	VuOglesGfx::IF()->bindIndexBuffer(mGlBuffer);

	// resize buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, newCount*2, VUNULL, mGlUsage);

	// update size
	mCount = newCount;
}

//*****************************************************************************
void VuOglesIndexBuffer::setData(const VUUINT16 *pData, int count)
{
	VuOglesGfx::IF()->bindIndexBuffer(mGlBuffer);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, count*2, pData);
}

//*****************************************************************************
VuOglesIndexBuffer *VuOglesIndexBuffer::create(int count, VUUINT32 usageFlags)
{
	// allocate buffer
	VuOglesIndexBuffer *pIndexBuffer = new VuOglesIndexBuffer(count);

	// translate usage
	pIndexBuffer->mGlUsage = GL_STATIC_DRAW;
	if ( usageFlags & VUGFX_USAGE_DYNAMIC )
		pIndexBuffer->mGlUsage = GL_DYNAMIC_DRAW;

	VuOglesGfx::IF()->bindIndexBuffer(pIndexBuffer->mGlBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count*2, VUNULL, pIndexBuffer->mGlUsage);

	return pIndexBuffer;
}
