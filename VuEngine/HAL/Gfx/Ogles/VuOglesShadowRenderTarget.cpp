//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ogles ShadowRenderTarget interface class.
// 
//*****************************************************************************

#include "VuOglesShadowRenderTarget.h"
#include "VuOglesTexture.h"
#include "VuOglesGfx.h"


//*****************************************************************************
VuOglesShadowRenderTarget::VuOglesShadowRenderTarget(int width, int height, int count):
	VuShadowRenderTarget(width, height, count),
	mFramebuffers(count)
{
	mFramebuffers.resize(count);
}

//*****************************************************************************
VuOglesShadowRenderTarget::~VuOglesShadowRenderTarget()
{
	if ( !VuOglesGfx::IF()->getContextDestroyed() )
	{
		for ( int i = 0; i < getCount(); i++ )
			glDeleteFramebuffers(1, &mFramebuffers[i]);

		glDeleteTextures(1, &mGlTexture);
	}
}

//*****************************************************************************
VuOglesShadowRenderTarget *VuOglesShadowRenderTarget::create(int width, int height, int count)
{
	// create render target
	VuOglesShadowRenderTarget *pShadowRenderTarget = new VuOglesShadowRenderTarget(width, height, count);

	// create texture
	glGenTextures(1, &pShadowRenderTarget->mGlTexture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, pShadowRenderTarget->mGlTexture);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT16, width, height, count, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, VUNULL);

	// create framebuffers
	for ( int i = 0; i < count; i++ )
	{
		// create framebuffer
		glGenFramebuffers(1, &pShadowRenderTarget->mFramebuffers[i]);

		// attach to framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, pShadowRenderTarget->mFramebuffers[i]);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, pShadowRenderTarget->mGlTexture, 0, i);

		VuOglesGfx::IF()->checkFramebufferStatus();
	}

	// clean up
	glBindFramebuffer(GL_FRAMEBUFFER, VuOglesGfx::IF()->getDefaultFramebuffer());

	return pShadowRenderTarget;
}
