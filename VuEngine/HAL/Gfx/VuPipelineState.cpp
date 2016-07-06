//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PipelineState interface class.
// 
//*****************************************************************************

#include "VuPipelineState.h"
#include "VuShaderProgram.h"
#include "VuVertexDeclaration.h"


//*****************************************************************************
VuPipelineState::VuPipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params):
	mpShaderProgram(pSP),
	mpVertexDeclaration(pVD),
	mParams(params)
{
	mpShaderProgram->addRef();
	mpVertexDeclaration->addRef();
}

//*****************************************************************************
VuPipelineState::~VuPipelineState()
{
	mpShaderProgram->removeRef();
	mpVertexDeclaration->removeRef();
}
