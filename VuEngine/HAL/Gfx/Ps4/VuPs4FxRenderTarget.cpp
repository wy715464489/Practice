//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ps4 FxRenderTarget interface class.
// 
//*****************************************************************************

#include "VuPs4FxRenderTarget.h"
#include "VuPs4Texture.h"
#include "VuPs4Gfx.h"
#include "VuPs4GfxTypes.h"


//*****************************************************************************
VuPs4FxRenderTarget::VuPs4FxRenderTarget(int width, int height, VuGfxFormat format) :
	VuFxRenderTarget(width, height, format)
{
}

//*****************************************************************************
VuPs4FxRenderTarget::~VuPs4FxRenderTarget()
{
	mpColorTexture->removeRef();
}

//*****************************************************************************
VuTexture *VuPs4FxRenderTarget::getTexture()
{
	return mpColorTexture;
}

//*****************************************************************************
VuPs4FxRenderTarget *VuPs4FxRenderTarget::create(int width, int height, VuGfxFormat format)
{
	// create render target
	VuPs4FxRenderTarget *pRenderTarget = new VuPs4FxRenderTarget(width, height, format);

	// create color texture/target
	{
		// determine format
		Gnm::DataFormat ps4ColorFormat = VuPs4GfxTypes::convert(format);

		// create state
		VuTextureState state;
		state.mAddressU = VUGFX_ADDRESS_CLAMP;
		state.mAddressV = VUGFX_ADDRESS_CLAMP;
		state.mMagFilter = VUGFX_TEXF_LINEAR;
		state.mMinFilter = VUGFX_TEXF_LINEAR;
		state.mMipFilter = VUGFX_TEXF_NONE;

		// determine tile mode
		Gnm::TileMode tileMode;
		GpuAddress::computeSurfaceTileMode(&tileMode, GpuAddress::kSurfaceTypeColorTarget, ps4ColorFormat, 1);

		// create texture
		pRenderTarget->mpColorTexture = new VuPs4Texture(width, height, 1, state);
		Gnm::SizeAlign sizeAlign = pRenderTarget->mpColorTexture->mGnmTexture.initAs2d(width, height, 1, ps4ColorFormat, tileMode, Gnm::kNumFragments1);
		pRenderTarget->mpColorTexture->mpGnmMemory = VuPs4Gfx::IF()->garlicAllocator().allocate(sizeAlign);
		pRenderTarget->mpColorTexture->mGnmTexture.setBaseAddress(pRenderTarget->mpColorTexture->mpGnmMemory);

		// init render target
		pRenderTarget->mColorTarget.initFromTexture(&pRenderTarget->mpColorTexture->mGnmTexture, 0);
	}

	return pRenderTarget;
}
