//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the pipeline state interface class.
//
//*****************************************************************************

#pragma once

#import <Metal/Metal.h>

#include "VuEngine/HAL/Gfx/VuPipelineState.h"


class VuMetalPipelineState : public VuPipelineState
{
public:
	VuMetalPipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params);
	~VuMetalPipelineState();
	
	static VuMetalPipelineState	*create(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params);
	
	VUUINT64					mHash;
	id<MTLRenderPipelineState>	mMTLRenderPipelineState;
};
