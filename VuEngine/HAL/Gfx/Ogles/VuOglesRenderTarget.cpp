//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ogles RenderTarget interface class.
// 
//*****************************************************************************

#include "VuOglesRenderTarget.h"
#include "VuOglesTexture.h"
#include "VuOglesGfx.h"


//*****************************************************************************
VuOglesRenderTarget::VuOglesRenderTarget(int width, int height):
	VuRenderTarget(width, height)
{
	glGenFramebuffers(1, &mGlFramebuffer);
	glGenRenderbuffers(1, &mGlDepthRenderbuffer);
}

//*****************************************************************************
VuOglesRenderTarget::~VuOglesRenderTarget()
{
	if ( !VuOglesGfx::IF()->getContextDestroyed() )
	{
		glDeleteFramebuffers(1, &mGlFramebuffer);
		glDeleteRenderbuffers(1, &mGlDepthRenderbuffer);
	}
}

//*****************************************************************************
VuTexture *VuOglesRenderTarget::getColorTexture()
{
	return mpColorTexture;
}

//*****************************************************************************
void VuOglesRenderTarget::readPixels(VuArray<VUBYTE> &rgb)
{
	int width = getWidth();
	int height = getHeight();

	VuArray<VUBYTE> rgba(0);
	rgba.resize(width*height*4);

	glBindFramebuffer(GL_FRAMEBUFFER, mGlFramebuffer);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &rgba[0]);
	glBindFramebuffer(GL_FRAMEBUFFER, VuOglesGfx::IF()->getDefaultFramebuffer());

	rgb.resize(width*height*3);

	int srcPitch = width*4;
	VUBYTE *pDst = static_cast<VUBYTE *>(&rgb[0]);

	for ( int y = 0; y < height; y++ )
	{
		VUBYTE *pSrc = &rgba[(height-y-1)*srcPitch];
		for ( int x = 0; x < width; x++ )
		{
			pDst[0] = pSrc[0];
			pDst[1] = pSrc[1];
			pDst[2] = pSrc[2];
			pSrc += 4;
			pDst += 3;
		}
	}
}

//*****************************************************************************
VuOglesRenderTarget *VuOglesRenderTarget::create(int width, int height)
{
	GLenum oglFormat = GL_RGBA;
	GLenum oglType = GL_UNSIGNED_BYTE;

	// create color texture
	VuTextureState state;
	state.mAddressU = VUGFX_ADDRESS_CLAMP;
	state.mAddressV = VUGFX_ADDRESS_CLAMP;
	state.mMagFilter = VUGFX_TEXF_LINEAR;
	state.mMinFilter = VUGFX_TEXF_LINEAR;
	state.mMipFilter = VUGFX_TEXF_NONE;
	VuOglesTexture *pColorTexture = new VuOglesTexture(width, height, 1, state);
	pColorTexture->mbDynamic = true;
	pColorTexture->mGlFormat = oglFormat;
	pColorTexture->mGlType = oglType;

	glBindTexture(GL_TEXTURE_2D, pColorTexture->mGlTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, pColorTexture->mGlFormat, width, height, 0, pColorTexture->mGlFormat, pColorTexture->mGlType, VUNULL);

	// create render target
	VuOglesRenderTarget *pRenderTarget = new VuOglesRenderTarget(width, height);
	pRenderTarget->mpColorTexture = pColorTexture;

	// create depth surface
	glBindRenderbuffer(GL_RENDERBUFFER, pRenderTarget->mGlDepthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);

	// attach to framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, pRenderTarget->mGlFramebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pColorTexture->mGlTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pRenderTarget->mGlDepthRenderbuffer);

	VuOglesGfx::IF()->checkFramebufferStatus();
	
	// clean up
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, VuOglesGfx::IF()->getDefaultFramebuffer());

	return pRenderTarget;
}
