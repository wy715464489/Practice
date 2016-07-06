//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d11 implementation of the depth-stencil state interface class.
// 
//*****************************************************************************

#include "VuD3d11DepthStencilState.h"
#include "VuD3d11Gfx.h"


// static variables

static struct D3d11DepthStencilStateData
{
	typedef std::hash_map<VUUINT32, VuD3d11DepthStencilState *> DepthStencilStates;
	DepthStencilStates mDepthStencilStates;
} sD3d11DepthStencilStateData;



//*****************************************************************************
VuD3d11DepthStencilState::VuD3d11DepthStencilState(const VuDepthStencilStateParams &params):
	VuDepthStencilState(params)
{
}

//*****************************************************************************
VuD3d11DepthStencilState::~VuD3d11DepthStencilState()
{
	sD3d11DepthStencilStateData.mDepthStencilStates.erase(mHash);
}

//*****************************************************************************
VuD3d11DepthStencilState *VuD3d11DepthStencilState::create(const VuDepthStencilStateParams &params)
{
	VUUINT32 hash = params.calcHash();

	D3d11DepthStencilStateData::DepthStencilStates::const_iterator iter = sD3d11DepthStencilStateData.mDepthStencilStates.find(hash);
	if ( iter != sD3d11DepthStencilStateData.mDepthStencilStates.end() )
	{
		iter->second->addRef();
		return iter->second;
	}

	// create new depth-stencil state
	VuD3d11DepthStencilState *pDepthStencilState = new VuD3d11DepthStencilState(params);
	pDepthStencilState->mHash = hash;

	CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
	desc.DepthEnable = (params.mDepthCompFunc != VUGFX_COMP_ALWAYS);
	desc.DepthWriteMask = params.mDepthWriteEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = VuD3d11GfxTypes::convert(params.mDepthCompFunc);

	D3DCALL(VuD3d11Gfx::IF()->getD3dDevice()->CreateDepthStencilState(&desc, &pDepthStencilState->mpD3dDepthStencilState));

	sD3d11DepthStencilStateData.mDepthStencilStates[hash] = pDepthStencilState;

	return pDepthStencilState;
}
