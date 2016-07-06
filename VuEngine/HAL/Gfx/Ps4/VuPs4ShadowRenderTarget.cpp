//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 ShadowRenderTarget interface class.
// 
//*****************************************************************************

#include "toolkit/toolkit.h"

#include "VuPs4ShadowRenderTarget.h"
#include "VuPs4Gfx.h"
#include "VuPs4Texture.h"


//*****************************************************************************
VuPs4ShadowRenderTarget::VuPs4ShadowRenderTarget(int width, int height, int count):
	VuShadowRenderTarget(width, height, count)
{
}

//*****************************************************************************
VuPs4ShadowRenderTarget::~VuPs4ShadowRenderTarget()
{
	VuPs4Gfx::IF()->garlicAllocator().release(mpGnmHtileMemory);
	VuPs4Gfx::IF()->garlicAllocator().release(mpGnmDepthMemory);
}

//*****************************************************************************
void VuPs4ShadowRenderTarget::resolve(int layer)
{
	Gnmx::GfxContext &gfxc = VuPs4Gfx::IF()->getCurGfxContext();

	int numSlices = 1;
	mDepthTarget.setArrayView(layer, layer);

	// Decompress shadow surfaces before we sample from them
	Gnmx::Toolkit::SurfaceUtil::decompressDepthSurface(gfxc, &mDepthTarget);
	Gnm::DbRenderControl dbRenderControl;
	dbRenderControl.init();
	gfxc.setDbRenderControl(dbRenderControl);
	gfxc.waitForGraphicsWrites(mDepthTarget.getZReadAddress256ByteBlocks(), (mDepthTarget.getZSliceSizeInBytes()*numSlices) >> 8, Gnm::kWaitTargetSlotDb,
		Gnm::kCacheActionWriteBackAndInvalidateL1andL2, Gnm::kExtendedCacheActionFlushAndInvalidateDbCache,
		Gnm::kStallCommandBufferParserDisable);
}

//*****************************************************************************
VuPs4ShadowRenderTarget *VuPs4ShadowRenderTarget::create(int width, int height, int count)
{
	// create shadow render target
	VuPs4ShadowRenderTarget *pRenderTarget = new VuPs4ShadowRenderTarget(width, height, count);

	// determine format
	Gnm::DataFormat depthFormat = Gnm::DataFormat::build(Gnm::kZFormat32Float);

	// determine tile mode
	Gnm::TileMode depthTileMode;
	GpuAddress::computeSurfaceTileMode(&depthTileMode, GpuAddress::kSurfaceTypeDepthOnlyTarget, depthFormat, 1);

	// allocate
	Gnm::SizeAlign htileSizeAlign;
	Gnm::SizeAlign depthTargetSizeAlign = pRenderTarget->mDepthTarget.init(width, height, count, depthFormat.getZFormat(), Gnm::kStencilInvalid, depthTileMode, Gnm::kNumFragments1, NULL, &htileSizeAlign);
	pRenderTarget->mpGnmHtileMemory = VuPs4Gfx::IF()->garlicAllocator().allocate(htileSizeAlign);
	pRenderTarget->mpGnmDepthMemory = VuPs4Gfx::IF()->garlicAllocator().allocate(depthTargetSizeAlign);
	pRenderTarget->mDepthTarget.setHtileAddress(pRenderTarget->mpGnmHtileMemory);
	pRenderTarget->mDepthTarget.setAddresses(pRenderTarget->mpGnmDepthMemory, NULL);

	// create texture
	pRenderTarget->mGnmTexture.initFromDepthRenderTarget(&pRenderTarget->mDepthTarget, false);

	// create sampler
	pRenderTarget->mGnmSampler.init();
	pRenderTarget->mGnmSampler.setDepthCompareFunction(Gnm::kDepthCompareLessEqual);
	pRenderTarget->mGnmSampler.setWrapMode(Gnm::kWrapModeClampLastTexel, Gnm::kWrapModeClampLastTexel, Gnm::kWrapModeClampLastTexel);
	pRenderTarget->mGnmSampler.setMipFilterMode(Gnm::kMipFilterModeNone);
	pRenderTarget->mGnmSampler.setXyFilterMode(Gnm::kFilterModeBilinear, Gnm::kFilterModeBilinear);

	return pRenderTarget;
}
