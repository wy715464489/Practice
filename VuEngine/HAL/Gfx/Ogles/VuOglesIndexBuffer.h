//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpenGL ES implementation of the index buffer interface class.
// 
//*****************************************************************************

#pragma once

#include "VuOglesIncl.h"

#include "VuEngine/HAL/Gfx/VuIndexBuffer.h"


class VuOglesIndexBuffer : public VuIndexBuffer
{
public:
	VuOglesIndexBuffer(int count);
	~VuOglesIndexBuffer();

	virtual void			resize(int newCount);
	virtual void			setData(const VUUINT16 *pData, int count);
	virtual const VUUINT16	*getShadowBuffer() { return VUNULL; }

	static VuOglesIndexBuffer	*create(int count, VUUINT32 usageFlags);

	GLuint					mGlBuffer;
	GLenum					mGlUsage;
};
