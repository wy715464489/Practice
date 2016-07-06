//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ogles DepthRenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#include "VuOglesIncl.h"
#include "VuEngine/HAL/Gfx/VuDepthRenderTarget.h"

class VuOglesTexture;


class VuOglesDepthRenderTarget : public VuDepthRenderTarget
{
public:
	VuOglesDepthRenderTarget(int width, int height);
	~VuOglesDepthRenderTarget();

	virtual VuTexture		*getTexture();

	static VuOglesDepthRenderTarget *create(int width, int height);

	VuOglesTexture	*mpTexture;
	GLuint			mGlFramebuffer;
};
