//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the cube texture interface class.
//
//*****************************************************************************

#pragma once

#import <Metal/Metal.h>

#include "VuEngine/HAL/Gfx/VuTexture.h"

class VuMetalSamplerState;


class VuMetalCubeTexture : public VuCubeTexture
{
public:
	VuMetalCubeTexture(int edgeLength, int levelCount, const VuTextureState &state);
	~VuMetalCubeTexture();
	
	static VuMetalCubeTexture	*load(VuBinaryDataReader &reader, int skipLevels);

	id<MTLTexture>		mMTLCubeTexture;
	VuMetalSamplerState	*mpSamplerState;
};
