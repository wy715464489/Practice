//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal RenderTarget interface class.
//
//*****************************************************************************

#pragma once

#import <Metal/Metal.h>

#include "VuEngine/HAL/Gfx/VuRenderTarget.h"

class VuMetalTexture;


class VuMetalRenderTarget : public VuRenderTarget
{
public:
	VuMetalRenderTarget(int width, int height);
	~VuMetalRenderTarget();
	
	virtual VuTexture	*getColorTexture();
	virtual void		readPixels(VuArray<VUBYTE> &rgb);
	
	static VuMetalRenderTarget *create(int width, int height);

	VuMetalTexture		*mpColorTexture;
	id <MTLTexture>		mDepthTexture;
};
