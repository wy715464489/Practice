//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android OpenGL ES interface class for Gfx.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Gfx/Ogles/VuOglesGfx.h"

class VuGfxDisplay;


class VuAndroidOglesGfx : public VuOglesGfx
{
public:
	VuAndroidOglesGfx();

	virtual bool		init(VUHANDLE hWindow, VUHANDLE hDevice);

	// thread ownership (not required)
	virtual void		acquireThreadOwnership();
	virtual void		releaseThreadOwnership();

	// display size
	virtual void		resize(VUHANDLE hDisplay, int width, int height);
	virtual void		getDisplaySize(VUHANDLE hDisplay, int &width, int &height);

	// render targets
	virtual void		setRenderTarget(const VuSetRenderTargetParams &params);

	// additional displays (not required)
	virtual VUHANDLE	createDisplay(VUHANDLE hWnd);
	virtual void		releaseDisplay(VUHANDLE hDisplay);

	// begin/end
	virtual bool		beginScene(VUHANDLE hDisplay);
	virtual bool		endScene(VUHANDLE hDisplay);

	// platform-specific functionality
	static VuAndroidOglesGfx *IF() { return static_cast<VuAndroidOglesGfx *>(VuOglesGfx::IF()); }

	// displays
	void				setDisplayData(VUHANDLE hDisplay, EGLDisplay display, EGLSurface surface, EGLContext context);

private:
	EGLDisplay			mDisplay;
	EGLSurface			mSurface;
	EGLContext			mContext;
	VuGfxDisplay		*mpCurDisplay;


	// discard framebuffer extension
	typedef void (GL_APIENTRYP DiscardFramebufferEXT)(GLenum target, GLsizei numAttachments, const GLenum *attachments);
	DiscardFramebufferEXT	glDiscardFramebufferEXT;
};
