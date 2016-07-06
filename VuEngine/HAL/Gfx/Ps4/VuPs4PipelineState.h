//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ps4 implementation of the pipeline state interface class.
// 
//*****************************************************************************

#pragma once

#include <gnm.h>
#include "VuEngine/HAL/Gfx/VuPipelineState.h"

using namespace sce;

class VuPs4ShaderProgram;


class VuPs4PipelineState : public VuPipelineState
{
public:
	VuPs4PipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params);
	~VuPs4PipelineState();

	static VuPs4PipelineState	*create(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params);

	VUUINT64			mHash;
	Gnm::BlendControl	mBlendControl;
	VUUINT32			mRenderTargetMask;
};
