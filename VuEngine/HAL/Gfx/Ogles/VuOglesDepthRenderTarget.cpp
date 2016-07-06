//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ogles DepthRenderTarget interface class.
// 
//*****************************************************************************

#include "VuOglesDepthRenderTarget.h"
#include "VuOglesTexture.h"
#include "VuOglesGfx.h"


// static variables


//*****************************************************************************
VuOglesDepthRenderTarget::VuOglesDepthRenderTarget(int width, int height):
	VuDepthRenderTarget(width, height)
{
}

//*****************************************************************************
VuOglesDepthRenderTarget::~VuOglesDepthRenderTarget()
{
	if ( !VuOglesGfx::IF()->getContextDestroyed() )
		glDeleteFramebuffers(1, &mGlFramebuffer);

	mpTexture->removeRef();
}

//*****************************************************************************
VuTexture *VuOglesDepthRenderTarget::getTexture()
{
	return mpTexture; 
}

//*****************************************************************************
VuOglesDepthRenderTarget *VuOglesDepthRenderTarget::create(int width, int height)
{
	// create render target
	VuOglesDepthRenderTarget *pDepthRenderTarget = new VuOglesDepthRenderTarget(width, height);

	// create texture
	VuTextureState state;
	state.mAddressU = VUGFX_ADDRESS_CLAMP;
	state.mAddressV = VUGFX_ADDRESS_CLAMP;
	state.mMagFilter = VUGFX_TEXF_LINEAR;
	state.mMinFilter = VUGFX_TEXF_LINEAR;
	state.mMipFilter = VUGFX_TEXF_NONE;

	VuOglesTexture *pTexture = new VuOglesTexture(width, height, 1, state);

	pTexture->mbDynamic = true;
	pTexture->mGlFormat = GL_DEPTH_COMPONENT;
	pTexture->mGlType = GL_UNSIGNED_SHORT;

	glBindTexture(GL_TEXTURE_2D, pTexture->mGlTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, pTexture->mGlFormat, width, height, 0, pTexture->mGlFormat, pTexture->mGlType, VUNULL);

	pDepthRenderTarget->mpTexture = pTexture;

	// create framebuffer
	glGenFramebuffers(1, &pDepthRenderTarget->mGlFramebuffer);

	// attach to framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, pDepthRenderTarget->mGlFramebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, pTexture->mGlTexture, 0);

	VuOglesGfx::IF()->checkFramebufferStatus();

	// clean up
	glBindFramebuffer(GL_FRAMEBUFFER, VuOglesGfx::IF()->getDefaultFramebuffer());

	return pDepthRenderTarget;
}
