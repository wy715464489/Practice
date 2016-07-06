//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ps4 FxRenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#include <gnm.h>
#include "VuEngine/HAL/Gfx/VuFxRenderTarget.h"

class VuPs4Texture;

using namespace sce;


class VuPs4FxRenderTarget : public VuFxRenderTarget
{
public:
	VuPs4FxRenderTarget(int width, int height, VuGfxFormat format);
	~VuPs4FxRenderTarget();

	virtual VuTexture		*getTexture();

	static VuPs4FxRenderTarget *create(int width, int height, VuGfxFormat format);

	VuPs4Texture			*mpColorTexture;

	Gnm::RenderTarget		mColorTarget;
};
