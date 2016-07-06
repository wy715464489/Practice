//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ogles RenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#include "VuOglesIncl.h"

#include "VuEngine/HAL/Gfx/VuRenderTarget.h"

class VuOglesTexture;


class VuOglesRenderTarget : public VuRenderTarget
{
public:
	VuOglesRenderTarget(int width, int height);
	~VuOglesRenderTarget();

	virtual VuTexture	*getColorTexture();
	virtual void		readPixels(VuArray<VUBYTE> &rgb);

	static VuOglesRenderTarget *create(int width, int height);

	GLuint				mGlFramebuffer;
	GLuint				mGlDepthRenderbuffer;
	VuOglesTexture		*mpColorTexture;
};
