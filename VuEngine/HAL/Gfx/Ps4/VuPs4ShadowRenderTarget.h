//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 ShadowRenderTarget interface class.
// 
//*****************************************************************************

#pragma once

#include <gnm.h>
#include "VuEngine/HAL/Gfx/VuShadowRenderTarget.h"
#include "VuEngine/Containers/VuArray.h"

class VuPs4Texture;

using namespace sce;


class VuPs4ShadowRenderTarget : public VuShadowRenderTarget
{
public:
	VuPs4ShadowRenderTarget(int width, int height, int count);
	~VuPs4ShadowRenderTarget();

	virtual void			resolve(int layer);

	static VuPs4ShadowRenderTarget *create(int width, int height, int count);

	Gnm::Texture			mGnmTexture;
	Gnm::DepthRenderTarget	mDepthTarget;
	void					*mpGnmHtileMemory;
	void					*mpGnmDepthMemory;
	Gnm::Sampler			mGnmSampler;
};
