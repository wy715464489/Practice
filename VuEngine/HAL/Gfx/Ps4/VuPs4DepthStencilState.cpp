//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ps4 implementation of the depth-stencil state interface class.
// 
//*****************************************************************************

#include "VuPs4DepthStencilState.h"
#include "VuPs4GfxTypes.h"


// static variables

static struct Ps4DepthStencilStateData
{
	typedef std::hash_map<VUUINT32, VuPs4DepthStencilState *> DepthStencilStates;
	DepthStencilStates mDepthStencilStates;
} sPs4DepthStencilStateData;



//*****************************************************************************
VuPs4DepthStencilState::VuPs4DepthStencilState(const VuDepthStencilStateParams &params):
	VuDepthStencilState(params)
{
}

//*****************************************************************************
VuPs4DepthStencilState::~VuPs4DepthStencilState()
{
	sPs4DepthStencilStateData.mDepthStencilStates.erase(mHash);
}

//*****************************************************************************
VuPs4DepthStencilState *VuPs4DepthStencilState::create(const VuDepthStencilStateParams &params)
{
	VUUINT32 hash = params.calcHash();

	Ps4DepthStencilStateData::DepthStencilStates::const_iterator iter = sPs4DepthStencilStateData.mDepthStencilStates.find(hash);
	if ( iter != sPs4DepthStencilStateData.mDepthStencilStates.end() )
	{
		iter->second->addRef();
		return iter->second;
	}

	// create new depth-stencil state
	VuPs4DepthStencilState *pDepthStencilState = new VuPs4DepthStencilState(params);
	pDepthStencilState->mHash = hash;

	Gnm::CompareFunc depthFunc = VuPs4GfxTypes::convert(params.mDepthCompFunc);
	Gnm::DepthControlZWrite zWrite = params.mDepthWriteEnabled ? Gnm::kDepthControlZWriteEnable : Gnm::kDepthControlZWriteDisable;

	pDepthStencilState->mGnmDSC.init();
	pDepthStencilState->mGnmDSC.setDepthControl(zWrite, depthFunc);
	pDepthStencilState->mGnmDSC.setDepthEnable(params.mDepthCompFunc != VUGFX_COMP_ALWAYS);

	sPs4DepthStencilStateData.mDepthStencilStates[hash] = pDepthStencilState;

	return pDepthStencilState;
}
