//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal sampler state helper class.
//
//*****************************************************************************

#pragma once

#import <Metal/Metal.h>

#include "VuEngine/HAL/Gfx/VuTexture.h"


class VuMetalSamplerState : public VuRefObj
{
public:
	~VuMetalSamplerState();
	
	static VuMetalSamplerState *create(const VuTextureState &state);
	
	VUUINT32			mHash;
	id<MTLSamplerState>	mMTLSamplerState;
};
