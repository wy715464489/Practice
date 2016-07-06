//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 sampler helper class.
// 
//*****************************************************************************

#include "VuPs4Sampler.h"


Gnm::WrapMode sAddressModeLookup[] =
{
	Gnm::kWrapModeWrap,           // VUGFX_ADDRESS_WRAP,
	Gnm::kWrapModeClampLastTexel, // VUGFX_ADDRESS_CLAMP,
};
VU_COMPILE_TIME_ASSERT(sizeof(sAddressModeLookup)/sizeof(sAddressModeLookup[0]) == VUGFX_TEXTURE_ADDRESS_COUNT);

Gnm::FilterMode sMinMagFilterLookup[] =
{
	Gnm::kFilterModePoint,      // VUGFX_TEXF_NONE,
	Gnm::kFilterModePoint,      // VUGFX_TEXF_POINT,
	Gnm::kFilterModeBilinear,   // VUGFX_TEXF_LINEAR,
	Gnm::kFilterModeAnisoPoint, // VUGFX_TEXF_ANISOTROPIC,
};
VU_COMPILE_TIME_ASSERT(sizeof(sMinMagFilterLookup)/sizeof(sMinMagFilterLookup[0]) == VUGFX_TEXTURE_FILTER_TYPE_COUNT);

Gnm::MipFilterMode sMipFilterLookup[] =
{
	Gnm::kMipFilterModeNone,   // VUGFX_TEXF_NONE,
	Gnm::kMipFilterModePoint,  // VUGFX_TEXF_POINT,
	Gnm::kMipFilterModeLinear, // VUGFX_TEXF_LINEAR,
	Gnm::kMipFilterModeLinear, // VUGFX_TEXF_ANISOTROPIC,
};
VU_COMPILE_TIME_ASSERT(sizeof(sMipFilterLookup)/sizeof(sMipFilterLookup[0]) == VUGFX_TEXTURE_FILTER_TYPE_COUNT);


//*****************************************************************************
void VuPs4Sampler::createSampler(const VuTextureState &state, Gnm::Sampler &sampler)
{
	sampler.init();
	sampler.setWrapMode(sAddressModeLookup[state.mAddressU], sAddressModeLookup[state.mAddressV], Gnm::kWrapModeWrap);
	sampler.setXyFilterMode(sMinMagFilterLookup[state.mMagFilter], sMinMagFilterLookup[state.mMinFilter]);
	sampler.setMipFilterMode(sMipFilterLookup[state.mMipFilter]);
}
