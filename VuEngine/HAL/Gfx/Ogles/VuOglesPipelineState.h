//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ogles implementation of the pipeline state interface class.
// 
//*****************************************************************************

#pragma once

#include "VuOglesIncl.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"

class VuOglesShaderProgram;


class VuOglesPipelineState : public VuPipelineState
{
public:
	VuOglesPipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params);
	~VuOglesPipelineState();

	static VuOglesPipelineState	*create(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params);

	VUUINT64				mHash;
	VuOglesShaderProgram	*mpOglesShaderProgram; // convenience
	GLenum					mOglesSrcBlendFactor;
	GLenum					mOglesDstBlendFactor;
};
