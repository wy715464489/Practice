//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ps4 DepthRenderTarget interface class.
// 
//*****************************************************************************

#include "VuPs4DepthRenderTarget.h"
#include "VuPs4Texture.h"
#include "VuPs4Gfx.h"


//*****************************************************************************
VuPs4DepthRenderTarget::VuPs4DepthRenderTarget(int width, int height) :
	VuDepthRenderTarget(width, height)
{
}

//*****************************************************************************
VuPs4DepthRenderTarget::~VuPs4DepthRenderTarget()
{
	mpColorTexture->removeRef();

	VuPs4Gfx::IF()->garlicAllocator().release(mpGnmHtileMemory);
	VuPs4Gfx::IF()->garlicAllocator().release(mpGnmDepthMemory);
}

//*****************************************************************************
VuTexture *VuPs4DepthRenderTarget::getTexture()
{
	return mpColorTexture;
}

//*****************************************************************************
VuPs4DepthRenderTarget *VuPs4DepthRenderTarget::create(int width, int height)
{
	// create render target
	VuPs4DepthRenderTarget *pRenderTarget = new VuPs4DepthRenderTarget(width, height);

	// create color texture/target
	{
		// determine format
		Gnm::DataFormat ps4ColorFormat = Gnm::kDataFormatR32Float;

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

	// create depth stencil surface
	{
		// determine format
		Gnm::DataFormat depthFormat = Gnm::DataFormat::build(Gnm::kZFormat32Float);

		// determine tile mode
		Gnm::TileMode depthTileMode;
		GpuAddress::computeSurfaceTileMode(&depthTileMode, GpuAddress::kSurfaceTypeDepthOnlyTarget, depthFormat, 1);

		// allocate
		Gnm::SizeAlign htileSizeAlign;
		Gnm::SizeAlign depthTargetSizeAlign = pRenderTarget->mDepthTarget.init(width, height, depthFormat.getZFormat(), Gnm::kStencilInvalid, depthTileMode, Gnm::kNumFragments1, NULL, &htileSizeAlign);
		pRenderTarget->mpGnmHtileMemory = VuPs4Gfx::IF()->garlicAllocator().allocate(htileSizeAlign);
		pRenderTarget->mpGnmDepthMemory = VuPs4Gfx::IF()->garlicAllocator().allocate(depthTargetSizeAlign);
		pRenderTarget->mDepthTarget.setHtileAddress(pRenderTarget->mpGnmHtileMemory);
		pRenderTarget->mDepthTarget.setAddresses(pRenderTarget->mpGnmDepthMemory, NULL);
	}

	return pRenderTarget;
}
