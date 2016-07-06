//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 implementation of the cube texture interface class.
// 
//*****************************************************************************

#pragma once

#include <gnm.h>
#include "VuEngine/HAL/Gfx/VuTexture.h"

using namespace sce;

class VuPs4CubeTexture : public VuCubeTexture
{
public:
	VuPs4CubeTexture(int edgeLength, int levelCount, const VuTextureState &state);
	~VuPs4CubeTexture();

	static VuPs4CubeTexture	*load(VuBinaryDataReader &reader, int skipLevels);

	Gnm::Texture	mGnmTexture;
	Gnm::Sampler	mGnmSampler;
	VUBYTE			*mpGnmMemory;
};
