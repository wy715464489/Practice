//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal ShadowRenderTarget interface class.
//
//*****************************************************************************

#include "VuMetalShadowRenderTarget.h"
#include "VuMetalTexture.h"
#include "VuMetalGfx.h"


//*****************************************************************************
VuMetalShadowRenderTarget::VuMetalShadowRenderTarget(int width, int height, int count):
	VuShadowRenderTarget(width, height, count)
{
}

//*****************************************************************************
VuMetalShadowRenderTarget::~VuMetalShadowRenderTarget()
{
}

//*****************************************************************************
VuMetalShadowRenderTarget *VuMetalShadowRenderTarget::create(int width, int height, int count)
{
	VuMetalShadowRenderTarget *pShadowRenderTarget = new VuMetalShadowRenderTarget(width, height, count);
	
	// create texture
	MTLTextureDescriptor *desc = [MTLTextureDescriptor
								  texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
								  width:width
								  height:height
								  mipmapped:NO];
	desc.textureType = MTLTextureType2DArray;
	desc.arrayLength = count;
	pShadowRenderTarget->mMTLTexture = [VuMetalGfx::getDevice() newTextureWithDescriptor:desc];
	
	return pShadowRenderTarget;
}
