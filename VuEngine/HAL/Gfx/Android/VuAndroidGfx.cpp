//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android OpenGL ES interface class for Gfx.
//
//*****************************************************************************

#include "VuAndroidGfx.h"
#include "VuEngine/HAL/Gfx/Ogles/VuOglesRenderTarget.h"
#include "VuEngine/VuEngine.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuGfx, VuAndroidOglesGfx);


class VuGfxDisplay
{
public:
	VuGfxDisplay() : mWidth(0), mHeight(0), mDisplay(VUNULL), mSurface(VUNULL), mContext(VUNULL) {}
	int			mWidth;
	int			mHeight;
	EGLDisplay	mDisplay;
	EGLSurface	mSurface;
	EGLContext	mContext;
};


//*****************************************************************************
VuAndroidOglesGfx::VuAndroidOglesGfx():
	mDisplay(VUNULL),
	mSurface(VUNULL),
	mContext(VUNULL),
	mpCurDisplay(VUNULL),
	glDiscardFramebufferEXT(VUNULL)
{
}

//*****************************************************************************
bool VuAndroidOglesGfx::init(VUHANDLE hWindow, VUHANDLE hDevice)
{
	if ( !VuOglesGfx::init(hWindow, hDevice) )
		return false;

	// init OpenGL ES 3 stub
	if ( gl3stubInit() )
	{
		VUPRINTF("GL3 stub init success\n");
	}
	else
	{
		VUPRINTF("GL3 stub init failed, going back to GL2\n");
		mGlVersion = 2;
	}

	if ( VuOglesGfx::IF()->getExtension("GL_EXT_discard_framebuffer") )
	{
		glDiscardFramebufferEXT = (DiscardFramebufferEXT)eglGetProcAddress("glDiscardFramebufferEXT");
		VUPRINTF("glDiscardFramebufferEXT: 0x%x\n", glDiscardFramebufferEXT);
	}

	return true;
}

//*****************************************************************************
void VuAndroidOglesGfx::acquireThreadOwnership()
{
	if ( eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, mContext) == EGL_FALSE )
		VUWARNING("Unable to eglMakeCurrent\n");
}

//*****************************************************************************
void VuAndroidOglesGfx::releaseThreadOwnership()
{
	if ( eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) == EGL_FALSE )
		VUWARNING("Unable to eglMakeCurrent\n");
}

//*****************************************************************************
void VuAndroidOglesGfx::resize(VUHANDLE hDisplay, int width, int height)
{
	VuGfxDisplay *pDisplay = (VuGfxDisplay *)hDisplay;

	if ( width > 0 && height > 0 )
	{
		if ( pDisplay )
		{
			pDisplay->mWidth = width;
			pDisplay->mHeight = height;
		}
		else
		{
			mDisplayWidth = width;
			mDisplayHeight = height;
		}

		mCurRenderTargetWidth = width;
		mCurRenderTargetHeight = height;
		mCurViewport = VuRect(0,0,1,1);
		glViewport(0, 0, width, height);
	}
}

//*****************************************************************************
void VuAndroidOglesGfx::getDisplaySize(VUHANDLE hDisplay, int &width, int &height)
{
	VuGfxDisplay *pDisplay = (VuGfxDisplay *)hDisplay;

	if ( pDisplay )
	{
		width = pDisplay->mWidth;
		height = pDisplay->mHeight;
	}
	else
	{
		width = mDisplayWidth;
		height = mDisplayHeight;
	}
}

//*****************************************************************************
void VuAndroidOglesGfx::setRenderTarget(const VuSetRenderTargetParams &params)
{
	if ( params.mpRenderTarget )
	{
		VuOglesRenderTarget *pOglesRenderTarget = (VuOglesRenderTarget *)params.mpRenderTarget;
		glBindFramebuffer(GL_FRAMEBUFFER, pOglesRenderTarget->mGlFramebuffer);

		checkFramebufferStatus();
	
		mCurRenderTargetWidth = pOglesRenderTarget->getWidth();
		mCurRenderTargetHeight = pOglesRenderTarget->getHeight();
	}
	else if ( mpCurDisplay )
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFramebuffer);

		mCurRenderTargetWidth = mpCurDisplay->mWidth;
		mCurRenderTargetHeight = mpCurDisplay->mHeight;
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
	if ( glDiscardFramebufferEXT )
	{
		GLenum discards[2];
		int discardsCount = 0;
		if ( params.mColorLoadAction == VuSetRenderTargetParams::LoadActionDontCare )
			discards[discardsCount++] = GL_COLOR_ATTACHMENT0;
		if ( params.mDepthLoadAction == VuSetRenderTargetParams::LoadActionDontCare )
			discards[discardsCount++] = GL_DEPTH_ATTACHMENT;

		if ( discardsCount )
			glDiscardFramebufferEXT(GL_FRAMEBUFFER, discardsCount, discards);
	}
}

//*****************************************************************************
VUHANDLE VuAndroidOglesGfx::createDisplay(VUHANDLE hWnd)
{
	VuGfxDisplay *pDisplay = new VuGfxDisplay;

	return pDisplay;
}

//*****************************************************************************
void VuAndroidOglesGfx::releaseDisplay(VUHANDLE hDisplay)
{
	VuGfxDisplay *pDisplay = (VuGfxDisplay *)hDisplay;

	delete pDisplay;
}

//*****************************************************************************
bool VuAndroidOglesGfx::beginScene(VUHANDLE hDisplay)
{
	VuGfx::beginScene(hDisplay);

	if ( mSyncGPU )
		glFinish();

	mpCurDisplay = (VuGfxDisplay *)hDisplay;

	if ( mpCurDisplay )
	{
		if ( eglMakeCurrent(mpCurDisplay->mDisplay, mpCurDisplay->mSurface, mpCurDisplay->mSurface, mpCurDisplay->mContext) == EGL_FALSE )
			VUWARNING("Unable to eglMakeCurrent\n");

		mCurRenderTargetWidth = mpCurDisplay->mWidth;
		mCurRenderTargetHeight = mpCurDisplay->mHeight;
	}
	else
	{
		if ( eglMakeCurrent(mDisplay, mSurface, mSurface, mContext) == EGL_FALSE )
			VUWARNING("Unable to eglMakeCurrent\n");

		mCurRenderTargetWidth = mDisplayWidth;
		mCurRenderTargetHeight = mDisplayHeight;
	}

	mCurViewport = VuRect(0,0,1,1);
	glViewport(0, 0, mCurRenderTargetWidth, mCurRenderTargetHeight);

	return true;
}

//*****************************************************************************
bool VuAndroidOglesGfx::endScene(VUHANDLE hDisplay)
{
	VUASSERT(hDisplay == mpCurDisplay, "VuGfx beginScene/endScene mismatch");

	VuGfx::endScene(hDisplay);

	// flip
	if ( mpCurDisplay )
		eglSwapBuffers(mpCurDisplay->mDisplay, mpCurDisplay->mSurface);
	else
		eglSwapBuffers(mDisplay, mSurface);

	mpCurDisplay = VUNULL;

	return true;
}

//*****************************************************************************
void VuAndroidOglesGfx::setDisplayData(VUHANDLE hDisplay, EGLDisplay display, EGLSurface surface, EGLContext context)
{
	VuGfxDisplay *pDisplay = (VuGfxDisplay *)hDisplay;

	if ( pDisplay )
	{
		pDisplay->mDisplay = display;
		pDisplay->mSurface = surface;
		pDisplay->mContext = context;
	}
	else
	{
		mDisplay = display;
		mSurface = surface;
		mContext = context;
	}
}