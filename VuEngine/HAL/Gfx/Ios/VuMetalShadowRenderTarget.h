//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal ShadowRenderTarget interface class.
//
//*****************************************************************************

#pragma once

#import <Metal/Metal.h>

#include "VuEngine/HAL/Gfx/VuShadowRenderTarget.h"


class VuMetalShadowRenderTarget : public VuShadowRenderTarget
{
public:
	VuMetalShadowRenderTarget(int width, int height, int count);
	~VuMetalShadowRenderTarget();
	
	static VuMetalShadowRenderTarget *create(int width, int height, int count);

	id <MTLTexture>		mMTLTexture;
};
