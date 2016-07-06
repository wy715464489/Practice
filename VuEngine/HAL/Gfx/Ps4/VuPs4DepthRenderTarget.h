//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ps4 DepthRenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#include <gnm.h>
#include "VuEngine/HAL/Gfx/VuDepthRenderTarget.h"

class VuPs4Texture;

using namespace sce;


class VuPs4DepthRenderTarget : public VuDepthRenderTarget
{
public:
	VuPs4DepthRenderTarget(int width, int height);
	~VuPs4DepthRenderTarget();

	virtual VuTexture		*getTexture();

	static VuPs4DepthRenderTarget *create(int width, int height);

	VuPs4Texture			*mpColorTexture;

	Gnm::RenderTarget		mColorTarget;
	Gnm::DepthRenderTarget	mDepthTarget;

	void					*mpGnmHtileMemory;
	void					*mpGnmDepthMemory;
};
