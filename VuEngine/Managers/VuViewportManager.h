//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ViewportManager class.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Math/VuRect.h"


class VuViewportManager : VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuViewportManager)

public:
	//*****************************************************************************
	// P U B L I C   M E T H O D S
	//*****************************************************************************

	VuViewportManager();
	~VuViewportManager();

protected:
	// called by engine
	friend class VuEngine;
	virtual bool	init();
	virtual void	release();

public:
	enum { MAX_VIEWPORTS = 8 };

	class VuViewport
	{
	public:
		VuViewport();

		VuRect		mUnsafeRect;
		VuRect		mRect;
		VuCamera	mCamera;

		// water surface z calculation
		float		mCurWaterSurfaceZ;
		float		mSrcWaterSurfaceZ;
		float		mDstWaterSurfaceZ;
		float		mOrigWaterSurfaceDist;

		float		mRadialBlurAmount;
	};

	void				reset();
	void				setViewportCount(int viewportCount);
	void				setCamera(int viewportIndex, const VuCamera &camera);

	int					getViewportCount()				{ return mViewportCount; }
	const VuViewport	&getViewport(int viewportIndex)	{ return mViewports[viewportIndex]; }

	void				setUiCameraHorz(float fFovHorz, float fAspectRatio, float fNear, float fFar);
	void				setUiCameraVert(float fFovVert, float fAspectRatio, float fNear, float fFar);
	const VuCamera		&getUiCamera() { return mUiCamera; }

	float				getSafeZone() { return mSafeZone; }
	const VuRect		&getSafeZoneRect() { return mSafeZoneRect; }

	// per-viewport effects
	void				resetEffects();
	void				setRadialBlur(int viewportIndex, float amount);

private:
	void				tickBuild(float fdt);
	void				draw();
	void				applySafeZone();
	void				finalize(VuRect &rect, int displayWidth, int displayHeight);

	int					mViewportCount;
	VuViewport			mViewports[MAX_VIEWPORTS];
	float				mSafeZone;
	VuRect				mSafeZoneRect;
	VuCamera			mUiCamera;
};
