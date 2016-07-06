//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ps4 implementation of the vertex buffer interface class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Gfx/VuVertexBuffer.h"


class VuPs4VertexBuffer : public VuVertexBuffer
{
public:
	VuPs4VertexBuffer(int size);
	~VuPs4VertexBuffer();

	virtual void			resize(int newSize);
	virtual void			setData(const void *pData, int size);
	virtual const void		*getShadowBuffer() { return VUNULL; }

	static VuPs4VertexBuffer	*create(int size, VUUINT32 usageFlags);

	void		*mpBuffer;
};
