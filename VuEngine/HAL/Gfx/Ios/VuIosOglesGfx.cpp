//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Ios Gfx Metal HAL.
//
//*****************************************************************************

#include "VuIosOglesGfx.h"
#include "VuOglesRenderTarget.h"


//*****************************************************************************
void VuIosOglesGfx::setRenderTarget(const VuSetRenderTargetParams &params)
{
	if ( params.mpRenderTarget )
	{
		VuOglesRenderTarget *pOglesRenderTarget = (VuOglesRenderTarget *)params.mpRenderTarget;
		glBindFramebuffer(GL_FRAMEBUFFER, pOglesRenderTarget->mGlFramebuffer);

		checkFramebufferStatus();
	
		mCurRenderTargetWidth = pOglesRenderTarget->getWidth();
		mCurRenderTargetHeight = pOglesRenderTarget->getHeight();
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFramebuffer);

		mCurRenderTargetWidth = mDisplayWidth;
		mCurRenderTargetHeight = mDisplayHeight;
	}

	mCurViewport = VuRect(0,0,1,1);
	glViewport(0, 0, mCurRenderTargetWidth, mCurRenderTargetHeight);

	for ( int i = 0; i < 8; i++ )
		VuGfx::IF()->setTexture(i, VUNULL);

	// handle clear
	GLbitfield clearMask = 0;
	if ( params.mColorLoadAction == VuSetRenderTargetParams::LoadActionClear )
	{
		clearMask |= GL_COLOR_BUFFER_BIT;
		VuVector4 vColor = params.mClearColor.toVector4();
		glClearColor(vColor.mX, vColor.mY, vColor.mZ, vColor.mW);
	}
	if ( params.mDepthLoadAction == VuSetRenderTargetParams::LoadActionClear )
	{
		clearMask |= GL_DEPTH_BUFFER_BIT;
		glClearDepthf(params.mClearDepth);
	}

	if ( clearMask )
		glClear(clearMask);

	// handle discard
	GLenum discards[2];
	int discardsCount = 0;
	if ( params.mColorLoadAction == VuSetRenderTargetParams::LoadActionDontCare )
		discards[discardsCount++] = GL_COLOR_ATTACHMENT0;
	if ( params.mDepthLoadAction == VuSetRenderTargetParams::LoadActionDontCare )
		discards[discardsCount++] = GL_DEPTH_ATTACHMENT;

	if ( discardsCount )
		glDiscardFramebufferEXT(GL_FRAMEBUFFER, discardsCount, discards);
}
