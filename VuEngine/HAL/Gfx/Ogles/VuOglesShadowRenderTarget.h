//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ogles ShadowRenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#include "VuOglesIncl.h"
#include "VuEngine/HAL/Gfx/VuShadowRenderTarget.h"
#include "VuEngine/Containers/VuArray.h"


class VuOglesShadowRenderTarget : public VuShadowRenderTarget
{
public:
	VuOglesShadowRenderTarget(int width, int height, int count);
	~VuOglesShadowRenderTarget();

	static VuOglesShadowRenderTarget *create(int width, int height, int count);

	typedef VuArray<GLuint> Framebuffers;

	GLuint			mGlTexture;
	Framebuffers	mFramebuffers;
};
