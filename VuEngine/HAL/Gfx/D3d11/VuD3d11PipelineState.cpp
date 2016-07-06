//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the pipeline state interface class.
// 
//*****************************************************************************

#include "VuD3d11PipelineState.h"
#include "VuD3d11ShaderProgram.h"
#include "VuD3d11VertexDeclaration.h"
#include "VuD3d11Gfx.h"
#include "VuEngine/Util/VuHash.h"
#include "VuEngine/Util/VuColor.h"


// static variables

static struct D3d11PipelineStateData
{
	typedef std::hash_map<VUUINT64, VuD3d11PipelineState *> PipelineStates;
	PipelineStates mPipelineStates;
} sD3d11PipelineStateData;



//*****************************************************************************
VuD3d11PipelineState::VuD3d11PipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params):
	VuPipelineState(pSP, pVD, params)
{
}

//*****************************************************************************
VuD3d11PipelineState::~VuD3d11PipelineState()
{
	sD3d11PipelineStateData.mPipelineStates.erase(mHash);
}

//*****************************************************************************
VuD3d11PipelineState *VuD3d11PipelineState::create(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params)
{
	// find a matchine pipeline state
	VUUINT64 hash = VuHash::fnv64(&params, sizeof(params));
	hash = VuHash::fnv64(&pSP, sizeof(pSP), hash);
	hash = VuHash::fnv64(&pVD, sizeof(pVD), hash);

	D3d11PipelineStateData::PipelineStates::const_iterator iter = sD3d11PipelineStateData.mPipelineStates.find(hash);
	if ( iter != sD3d11PipelineStateData.mPipelineStates.end() )
	{
		iter->second->addRef();
		return iter->second;
	}

	// create new pipeline state
	VuD3d11PipelineState *pPipelineState = new VuD3d11PipelineState(pSP, pVD, params);
	pPipelineState->mHash = hash;
	pPipelineState->mpD3dShaderProgram = static_cast<VuD3d11ShaderProgram *>(pSP);
	pPipelineState->mpD3dVertexShader = static_cast<VuD3d11ShaderProgram *>(pSP)->getD3d11VertexShader();
	pPipelineState->mpD3dPixelShader = static_cast<VuD3d11ShaderProgram *>(pSP)->getD3d11PixelShader();
	pPipelineState->mpD3dInputLayout = static_cast<VuD3d11VertexDeclaration *>(pVD)->mpD3dInputLayout;
	pPipelineState->mpD3dBlendState = VuD3d11Gfx::IF()->createBlendState(params);

	sD3d11PipelineStateData.mPipelineStates[hash] = pPipelineState;

	return pPipelineState;
}
