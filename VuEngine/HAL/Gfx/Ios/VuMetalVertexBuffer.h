//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the vertex buffer interface class.
//
//*****************************************************************************

#pragma once

#import <Metal/Metal.h>

#include "VuEngine/HAL/Gfx/VuVertexBuffer.h"


class VuMetalVertexBuffer : public VuVertexBuffer
{
public:
	VuMetalVertexBuffer(int size);
	~VuMetalVertexBuffer();
	
	virtual void		resize(int newSize);
	virtual void		setData(const void *pData, int size);
	virtual const void	*getShadowBuffer() { return VUNULL; }
	
	static VuMetalVertexBuffer	*create(int size, VUUINT32 usageFlags);
	
	id<MTLBuffer>	mMTLBuffer;
};
