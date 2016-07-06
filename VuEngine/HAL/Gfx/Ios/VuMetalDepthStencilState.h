//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the depth-stencil state interface class.
//
//*****************************************************************************

#pragma once

#import <Metal/Metal.h>

#include "VuEngine/HAL/Gfx/VuDepthStencilState.h"


class VuMetalDepthStencilState : public VuDepthStencilState
{
public:
	VuMetalDepthStencilState(const VuDepthStencilStateParams &params);
	~VuMetalDepthStencilState();
	
	static VuMetalDepthStencilState	*create(const VuDepthStencilStateParams &params);
	
	VUUINT32					mHash;
	id<MTLDepthStencilState>	mMTLDepthStencilState;
};
