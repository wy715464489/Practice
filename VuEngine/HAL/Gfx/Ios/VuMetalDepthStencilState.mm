//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the depth-stencil state interface class.
//
//*****************************************************************************

#include "VuMetalDepthStencilState.h"
#include "VuMetalGfx.h"


// static variables

static struct MetalDepthStencilStateData
{
	typedef std::hash_map<VUUINT32, VuMetalDepthStencilState *> DepthStencilStates;
	DepthStencilStates mDepthStencilStates;
} sMetalDepthStencilStateData;


MTLCompareFunction sDepthCompLookup[] =
{
	MTLCompareFunctionNever,        // VUGFX_COMP_NEVER,
	MTLCompareFunctionLess,         // VUGFX_COMP_LESS,
	MTLCompareFunctionEqual,        // VUGFX_COMP_EQUAL,
	MTLCompareFunctionLessEqual,    // VUGFX_COMP_LESSEQUAL,
	MTLCompareFunctionGreater,      // VUGFX_COMP_GREATER,
	MTLCompareFunctionNotEqual,     // VUGFX_COMP_NOTEQUAL,
	MTLCompareFunctionGreaterEqual, // VUGFX_COMP_GREATEREQUAL,
	MTLCompareFunctionAlways,       // VUGFX_COMP_ALWAYS,
};
VU_COMPILE_TIME_ASSERT(sizeof(sDepthCompLookup)/sizeof(sDepthCompLookup[0]) == VUGFX_COMP_FUNC_COUNT);


//*****************************************************************************
VuMetalDepthStencilState::VuMetalDepthStencilState(const VuDepthStencilStateParams &params):
	VuDepthStencilState(params)
{
}

//*****************************************************************************
VuMetalDepthStencilState::~VuMetalDepthStencilState()
{
	sMetalDepthStencilStateData.mDepthStencilStates.erase(mHash);
}

//*****************************************************************************
VuMetalDepthStencilState *VuMetalDepthStencilState::create(const VuDepthStencilStateParams &params)
{
	VUUINT32 hash = params.calcHash();
	
	MetalDepthStencilStateData::DepthStencilStates::const_iterator iter = sMetalDepthStencilStateData.mDepthStencilStates.find(hash);
	if ( iter != sMetalDepthStencilStateData.mDepthStencilStates.end() )
	{
		iter->second->addRef();
		return iter->second;
	}
	
	MTLDepthStencilDescriptor *pDescriptor = [[MTLDepthStencilDescriptor alloc] init];
	pDescriptor.depthCompareFunction = sDepthCompLookup[params.mDepthCompFunc];
	pDescriptor.depthWriteEnabled = params.mDepthWriteEnabled;
	
	id<MTLDepthStencilState> depthStencilState = [VuMetalGfx::getDevice() newDepthStencilStateWithDescriptor:pDescriptor];
	
	// create new depth-stencil state
	VuMetalDepthStencilState *pDepthStencilState = new VuMetalDepthStencilState(params);
	pDepthStencilState->mHash = hash;
	pDepthStencilState->mMTLDepthStencilState = depthStencilState;
	
	sMetalDepthStencilStateData.mDepthStencilStates[hash] = pDepthStencilState;
	
	return pDepthStencilState;
}
