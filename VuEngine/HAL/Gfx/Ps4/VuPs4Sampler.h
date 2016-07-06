//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 sampler helper class.
// 
//*****************************************************************************

#pragma once

#include <gnm.h>

#include "VuPs4Texture.h"

namespace VuPs4Sampler
{
	void	createSampler(const VuTextureState &state, Gnm::Sampler &sampler);
}