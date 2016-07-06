//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DefaultGfxComposer implementation class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Gfx/GfxComposer/VuGfxComposer.h"
#include "VuEngine/HAL/Keyboard/VuKeyboard.h"

class VuCamera;
class VuRenderTarget;
class VuGfxSettings;


class VuDefaultGfxComposer : VuGfxComposer::GameInterface, VuKeyboard::Callback
{
public:
	VuDefaultGfxComposer();
	~VuDefaultGfxComposer();

	bool				init();
	void				release();

	// VuGfxComposer::GameInterface
	virtual VuTexture	*getWaterReflectionTexture(int viewport);
	virtual VuTexture	*getDepthTexture();
	virtual VuVector2	getWaterReflectionMapOffset()	{ return mReflectionMapOffset; }
	virtual VuVector2	getWaterReflectionMapScale()	{ return mReflectionMapScale; }

protected:
	// VuKeyboard::Callback
	virtual void		onKeyDown(VUUINT32 key);

	void				draw();
	void				submitSceneCommands(VuRenderTarget *pRenderTarget, const VuCamera &camera, const VuGfxSettings &gfxSettings, VUUINT32 zoneMask);
	void				submitReflectionCommands(const VuCamera &camera, const VuGfxSettings &gfxSettings, VUUINT32 zoneMask);

	void				drawReflectionMap(VuTexture *pReflectionTexture);

	void				updateRenderTargets();
	void				destroyRenderTargets();

	void				takeScreenShot();

	// tweaks
	float				mDisplayScale;
	bool				mScaledUI;
	bool				mRadialBlurEnabled;
	bool				mColorCorrectionEnabled;
	bool				mDisablePresent;
	int					mOverrideDisplayWidth;
	int					mOverrideDisplayHeight;
	float				mRejectionScale;
	bool				mbDrawCollision;

	int					mCurRenderTargetWidth;
	int					mCurRenderTargetHeight;
	VuRenderTarget		*mpRenderTargets[2];
	bool				mDisplayIsScaled;

	// reflection
	float				mReflectionRenderTargetScale;
	float				mReflectionRejectionScale;
	bool				mbShowReflectionMap;
	VuRenderTarget		*mpReflectionRenderTarget;
	VuVector2			mReflectionMapOffset;
	VuVector2			mReflectionMapScale;

	int					mScreenShotSize;
};
