//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal RenderTarget interface class.
//
//*****************************************************************************

#include "VuMetalRenderTarget.h"
#include "VuMetalTexture.h"
#include "VuMetalSamplerState.h"
#include "VuMetalGfx.h"


//*****************************************************************************
VuMetalRenderTarget::VuMetalRenderTarget(int width, int height):
	VuRenderTarget(width, height)
{
}

//*****************************************************************************
VuMetalRenderTarget::~VuMetalRenderTarget()
{
}

//*****************************************************************************
VuTexture *VuMetalRenderTarget::getColorTexture()
{
	return mpColorTexture;
}

//*****************************************************************************
void VuMetalRenderTarget::readPixels(VuArray<VUBYTE> &rgb)
{
}

//*****************************************************************************
VuMetalRenderTarget *VuMetalRenderTarget::create(int width, int height)
{
	// create color texture
	VuTextureState state;
	state.mAddressU = VUGFX_ADDRESS_CLAMP;
	state.mAddressV = VUGFX_ADDRESS_CLAMP;
	state.mMagFilter = VUGFX_TEXF_LINEAR;
	state.mMinFilter = VUGFX_TEXF_LINEAR;
	state.mMipFilter = VUGFX_TEXF_NONE;
	VuMetalTexture *pColorTexture = new VuMetalTexture(width, height, 1);

	MTLTextureDescriptor *colorDesc = [MTLTextureDescriptor
									   texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
									   width:width
									   height:height
									   mipmapped:NO];
	pColorTexture->mMTLTexture = [VuMetalGfx::getDevice() newTextureWithDescriptor:colorDesc];
	pColorTexture->mpSamplerState = VuMetalSamplerState::create(state);

	// create depth texture
	MTLTextureDescriptor *depthDesc = [MTLTextureDescriptor
									   texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
									   width:width
									   height:height
									   mipmapped:NO];
	id <MTLTexture> depthTexture = [VuMetalGfx::getDevice() newTextureWithDescriptor:depthDesc];
	
	// create render target
	VuMetalRenderTarget *pRenderTarget = new VuMetalRenderTarget(width, height);
	pRenderTarget->mpColorTexture = pColorTexture;
	pRenderTarget->mDepthTexture = depthTexture;
	
	return pRenderTarget;
}
