//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ogles implementation of the pipeline state interface class.
// 
//*****************************************************************************

#include "VuOglesPipelineState.h"
#include "VuOglesShaderProgram.h"
#include "VuOglesGfxTypes.h"
#include "VuEngine/Util/VuHash.h"


// static variables

static struct OglesPipelineStateData
{
	typedef std::map<VUUINT64, VuOglesPipelineState *> PipelineStates;
	PipelineStates mPipelineStates;
} sOglesPipelineStateData;


//*****************************************************************************
VuOglesPipelineState::VuOglesPipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params):
	VuPipelineState(pSP, pVD, params)
{
}

//*****************************************************************************
VuOglesPipelineState::~VuOglesPipelineState()
{
	sOglesPipelineStateData.mPipelineStates.erase(mHash);
}

//*****************************************************************************
VuOglesPipelineState *VuOglesPipelineState::create(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params)
{
	// find a matchine pipeline state
	VUUINT64 hash = VuHash::fnv64(&params, sizeof(params));
	hash = VuHash::fnv64(&pSP, sizeof(pSP), hash);
	hash = VuHash::fnv64(&pVD, sizeof(pVD), hash);

	OglesPipelineStateData::PipelineStates::const_iterator iter = sOglesPipelineStateData.mPipelineStates.find(hash);
	if ( iter != sOglesPipelineStateData.mPipelineStates.end() )
	{
		iter->second->addRef();
		return iter->second;
	}

	// create new pipeline state
	VuOglesPipelineState *pPipelineState = new VuOglesPipelineState(pSP, pVD, params);
	pPipelineState->mHash = hash;
	pPipelineState->mpOglesShaderProgram = static_cast<VuOglesShaderProgram *>(pSP);
	pPipelineState->mOglesSrcBlendFactor = VuOglesGfxTypes::convert(params.mSrcBlendMode);
	pPipelineState->mOglesDstBlendFactor = VuOglesGfxTypes::convert(params.mDstBlendMode);

	sOglesPipelineStateData.mPipelineStates[hash] = pPipelineState;

	return pPipelineState;
}
