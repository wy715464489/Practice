//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpenGL ES implementation of the vertex buffer interface class.
// 
//*****************************************************************************

#pragma once

#include "VuOglesIncl.h"

#include "VuEngine/HAL/Gfx/VuVertexBuffer.h"


class VuOglesVertexBuffer : public VuVertexBuffer
{
public:
	VuOglesVertexBuffer(int size);
	~VuOglesVertexBuffer();

	virtual void		resize(int newSize);
	virtual void		setData(const void *pData, int size);
	virtual const void	*getShadowBuffer() { return VUNULL; }

	static VuOglesVertexBuffer	*create(int size, VUUINT32 usageFlags);

	GLuint				mGlBuffer;
	GLenum				mGlUsage;
};
