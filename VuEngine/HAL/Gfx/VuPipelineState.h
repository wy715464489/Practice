//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PipelineState interface class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuVertexDeclaration.h"


struct VuPipelineStateParams
{
	VuPipelineStateParams() :
		mAlphaBlendEnabled(false),
		mSrcBlendMode(VUGFX_BLEND_SRCALPHA),
		mDstBlendMode(VUGFX_BLEND_INVSRCALPHA),
		mColorWriteEnabled(true),
		mDepthRenderingHint(false)
	{}

	bool						mAlphaBlendEnabled;
	VuGfxBlendMode				mSrcBlendMode;
	VuGfxBlendMode				mDstBlendMode;
	bool						mColorWriteEnabled;
	bool						mDepthRenderingHint;
};

class VuPipelineState : public VuRefObj
{
public:
	VuPipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params);
	~VuPipelineState();

	VuShaderProgram			*mpShaderProgram;
	VuVertexDeclaration		*mpVertexDeclaration;
	VuPipelineStateParams	mParams;
};
