//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ps4 RenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#include <gnm.h>
#include "VuEngine/HAL/Gfx/VuRenderTarget.h"

class VuPs4Texture;

using namespace sce;


class VuPs4RenderTarget : public VuRenderTarget
{
public:
	VuPs4RenderTarget(int width, int height);
	~VuPs4RenderTarget();

	virtual VuTexture		*getColorTexture();
	virtual void			readPixels(VuArray<VUBYTE> &rgb);

	static VuPs4RenderTarget *create(int width, int height);

	VuPs4Texture			*mpColorTexture;

	Gnm::RenderTarget		mColorTarget;
	Gnm::DepthRenderTarget	mDepthTarget;

	void					*mpGnmHtileMemory;
	void					*mpGnmDepthMemory;
};
