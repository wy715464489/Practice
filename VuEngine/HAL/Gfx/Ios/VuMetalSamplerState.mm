//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal sampler state helper class.
//
//*****************************************************************************

#include "VuMetalSamplerState.h"
#include "VuMetalGfx.h"
#include "VuEngine/Util/VuHash.h"


// static variables

static struct MetalSamplerStateData
{
	typedef std::map<VUUINT32, VuMetalSamplerState *> SamplerStates;
	SamplerStates mSamplerStates;
} sMetalSamplerStateData;


MTLSamplerAddressMode sAddressModeLookup[] =
{
	MTLSamplerAddressModeRepeat,      // VUGFX_ADDRESS_WRAP,
	MTLSamplerAddressModeClampToEdge, // VUGFX_ADDRESS_CLAMP,
};
VU_COMPILE_TIME_ASSERT(sizeof(sAddressModeLookup)/sizeof(sAddressModeLookup[0]) == VUGFX_TEXTURE_ADDRESS_COUNT);

MTLSamplerMinMagFilter sMinMagFilterLookup[] =
{
	MTLSamplerMinMagFilterNearest, // VUGFX_TEXF_NONE,
	MTLSamplerMinMagFilterNearest, // VUGFX_TEXF_POINT,
	MTLSamplerMinMagFilterLinear,  // VUGFX_TEXF_LINEAR,
	MTLSamplerMinMagFilterLinear,  // VUGFX_TEXF_ANISOTROPIC,
};
VU_COMPILE_TIME_ASSERT(sizeof(sMinMagFilterLookup)/sizeof(sMinMagFilterLookup[0]) == VUGFX_TEXTURE_FILTER_TYPE_COUNT);

MTLSamplerMipFilter sMipFilterLookup[] =
{
	MTLSamplerMipFilterNotMipmapped, // VUGFX_TEXF_NONE,
	MTLSamplerMipFilterNearest,      // VUGFX_TEXF_POINT,
	MTLSamplerMipFilterLinear,       // VUGFX_TEXF_LINEAR,
	MTLSamplerMipFilterLinear,       // VUGFX_TEXF_ANISOTROPIC,
};
VU_COMPILE_TIME_ASSERT(sizeof(sMipFilterLookup)/sizeof(sMipFilterLookup[0]) == VUGFX_TEXTURE_FILTER_TYPE_COUNT);


//*****************************************************************************
VuMetalSamplerState::~VuMetalSamplerState()
{
	sMetalSamplerStateData.mSamplerStates.erase(mHash);
}

//*****************************************************************************
VuMetalSamplerState *VuMetalSamplerState::create(const VuTextureState &state)
{
	// calculate hash
	VUUINT32 hash = VuHash::fnv32(&state, sizeof(state));
	
	MetalSamplerStateData::SamplerStates::const_iterator iter = sMetalSamplerStateData.mSamplerStates.find(hash);
	if ( iter != sMetalSamplerStateData.mSamplerStates.end() )
	{
		iter->second->addRef();
		return iter->second;
	}

	MTLSamplerDescriptor *pDescriptor = [[MTLSamplerDescriptor alloc] init];
	pDescriptor.sAddressMode = sAddressModeLookup[state.mAddressU];
	pDescriptor.tAddressMode = sAddressModeLookup[state.mAddressV];
	pDescriptor.minFilter = sMinMagFilterLookup[state.mMinFilter];
	pDescriptor.magFilter = sMinMagFilterLookup[state.mMagFilter];
	pDescriptor.mipFilter = sMipFilterLookup[state.mMipFilter];
	
	id<MTLSamplerState> samplerState = [VuMetalGfx::getDevice() newSamplerStateWithDescriptor:pDescriptor];
	
	// create new sampler state
	VuMetalSamplerState *pSamplerState = new VuMetalSamplerState();
	pSamplerState->mHash = hash;
	pSamplerState->mMTLSamplerState = samplerState;
	
	sMetalSamplerStateData.mSamplerStates[hash] = pSamplerState;
	
	return pSamplerState;
}