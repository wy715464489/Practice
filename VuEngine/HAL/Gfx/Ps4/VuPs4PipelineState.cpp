//*****************************************************************************
//
//  Copyright (c) 2014-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ps4 implementation of the pipeline state interface class.
// 
//*****************************************************************************

#include "VuPs4PipelineState.h"
#include "VuPs4ShaderProgram.h"
#include "VuPs4VertexDeclaration.h"
#include "VuPs4Gfx.h"
#include "VuPs4GfxTypes.h"
#include "VuEngine/Util/VuHash.h"


// static variables

static struct Ps4PipelineStateData
{
	typedef std::hash_map<VUUINT64, VuPs4PipelineState *> PipelineStates;
	PipelineStates mPipelineStates;
} sPs4PipelineStateData;



//*****************************************************************************
VuPs4PipelineState::VuPs4PipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params):
	VuPipelineState(pSP, pVD, params)
{
}

//*****************************************************************************
VuPs4PipelineState::~VuPs4PipelineState()
{
	sPs4PipelineStateData.mPipelineStates.erase(mHash);
}

//*****************************************************************************
VuPs4PipelineState *VuPs4PipelineState::create(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params)
{
	// find a matchine pipeline state
	VUUINT64 hash = VuHash::fnv64(&params, sizeof(params));
	hash = VuHash::fnv64(&pSP, sizeof(pSP), hash);
	hash = VuHash::fnv64(&pVD, sizeof(pVD), hash);

	Ps4PipelineStateData::PipelineStates::const_iterator iter = sPs4PipelineStateData.mPipelineStates.find(hash);
	if ( iter != sPs4PipelineStateData.mPipelineStates.end() )
	{
		iter->second->addRef();
		return iter->second;
	}

	// create new pipeline state
	VuPs4PipelineState *pPipelineState = new VuPs4PipelineState(pSP, pVD, params);
	pPipelineState->mHash = hash;

	Gnm::BlendMultiplier srcMul = VuPs4GfxTypes::convert(params.mSrcBlendMode);
	Gnm::BlendMultiplier dstMul = VuPs4GfxTypes::convert(params.mDstBlendMode);

	pPipelineState->mBlendControl.init();
	pPipelineState->mBlendControl.setBlendEnable(params.mAlphaBlendEnabled);
	pPipelineState->mBlendControl.setColorEquation(srcMul, Gnm::kBlendFuncAdd, dstMul);

	pPipelineState->mRenderTargetMask = params.mColorWriteEnabled ? 0xf : 0x0;

	sPs4PipelineStateData.mPipelineStates[hash] = pPipelineState;

	return pPipelineState;
}
