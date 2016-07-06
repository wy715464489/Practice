//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal DepthRenderTarget interface class.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Gfx/VuDepthRenderTarget.h"

class VuMetalTexture;


class VuMetalDepthRenderTarget : public VuDepthRenderTarget
{
public:
	VuMetalDepthRenderTarget(int width, int height);
	~VuMetalDepthRenderTarget();
	
	virtual VuTexture		*getTexture();
	
	static VuMetalDepthRenderTarget *create(int width, int height);
};
