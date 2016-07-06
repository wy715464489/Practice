//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the index buffer interface class.
//
//*****************************************************************************

#pragma once

#import <Metal/Metal.h>

#include "VuEngine/HAL/Gfx/VuIndexBuffer.h"


class VuMetalIndexBuffer : public VuIndexBuffer
{
public:
	VuMetalIndexBuffer(int count);
	~VuMetalIndexBuffer();
	
	virtual void			resize(int newCount);
	virtual void			setData(const VUUINT16 *pData, int count);
	virtual const VUUINT16	*getShadowBuffer() { return VUNULL; }
	
	static VuMetalIndexBuffer	*create(int count, VUUINT32 usageFlags);

	id<MTLBuffer>	mMTLBuffer;
};
