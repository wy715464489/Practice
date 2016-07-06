//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ogles implementation of the depth-stencil state interface class.
// 
//*****************************************************************************

#include "VuOglesDepthStencilState.h"
#include "VuOglesGfxTypes.h"


// static variables

static struct OglesDepthStencilStateData
{
	typedef std::hash_map<VUUINT32, VuOglesDepthStencilState *> DepthStencilStates;
	DepthStencilStates mDepthStencilStates;
} sOglesDepthStencilStateData;



//*****************************************************************************
VuOglesDepthStencilState::VuOglesDepthStencilState(const VuDepthStencilStateParams &params):
	VuDepthStencilState(params)
{
}

//*****************************************************************************
VuOglesDepthStencilState::~VuOglesDepthStencilState()
{
	sOglesDepthStencilStateData.mDepthStencilStates.erase(mHash);
}

//*****************************************************************************
VuOglesDepthStencilState *VuOglesDepthStencilState::create(const VuDepthStencilStateParams &params)
{
	VUUINT32 hash = params.calcHash();

	OglesDepthStencilStateData::DepthStencilStates::const_iterator iter = sOglesDepthStencilStateData.mDepthStencilStates.find(hash);
	if ( iter != sOglesDepthStencilStateData.mDepthStencilStates.end() )
	{
		iter->second->addRef();
		return iter->second;
	}

	// create new depth-stencil state
	VuOglesDepthStencilState *pDepthStencilState = new VuOglesDepthStencilState(params);
	pDepthStencilState->mHash = hash;
	pDepthStencilState->mOglesDepthFunc = VuOglesGfxTypes::convert(params.mDepthCompFunc);

	sOglesDepthStencilStateData.mDepthStencilStates[hash] = pDepthStencilState;

	return pDepthStencilState;
}
