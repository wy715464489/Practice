//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DefaultGfxComposer implementation class
// 
//*****************************************************************************

#include "VuDefaultGfxComposer.h"
#include "VuGfxComposerCommands.h"
#include "VuEngine/Util/VuScreenShotUtil.h"
#include "VuEngine/VuEngine.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Managers/VuGfxSettingsManager.h"
#include "VuEngine/Managers/VuFoliageManager.h"
#include "VuEngine/Managers/VuLensWaterManager.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawManager.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuRenderTarget.h"
#include "VuEngine/Water/VuWater.h"
#include "VuEngine/Dynamics/VuDynamics.h"
#include "VuEngine/Math/VuMathUtil.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Util/VuImageUtil.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Dev/VuDevProfile.h"
#include "VuEngine/Dev/VuDevConfig.h"


struct ScreenShotMetrics
{
	int mWidth, mHeight, mRefWidth, mRefHeight, mCount;
};
static ScreenShotMetrics sScreenShotMetrics[] =
{
	{    0,    0,    0,   0,  0 }, // current size
	{ 1280,  720,  640, 360,  1 }, //   921600 pixels
	{ 1920, 1080,  960, 540,  2 }, //  2073600 pixels
	{ 2560, 1440, 1280, 720,  4 }, //  3686400 pixels
	{ 3840, 2160, 1280, 720,  8 }, //  8294400 pixels
	{ 5120, 2880, 1280, 720, 16 }, // 14745600 pixels
	{ 7680, 4320, 1280, 720, 32 }, // 33177600 pixels
};

// choices
static VuDevMenu::IntEnumChoice sScreenShotChoices[] =
{
	{ "Current", 0},
	{ "1280x720", 1},
	{ "1920x1080", 2},
	{ "2560x1440", 3},
	{ "3840x2160", 4},
	{ "5120x2880", 5},
	{ "7680x4320", 6},
	{ VUNULL }
};
VU_COMPILE_TIME_ASSERT(sizeof(sScreenShotChoices)/sizeof(sScreenShotChoices[0]) - 1 == sizeof(sScreenShotMetrics)/sizeof(sScreenShotMetrics[0]));


//*****************************************************************************
VuDefaultGfxComposer::VuDefaultGfxComposer():
	mDisplayScale(1.0f),
	mScaledUI(false),
	mRadialBlurEnabled(true),
	mColorCorrectionEnabled(true),
	mDisablePresent(false),
	mOverrideDisplayWidth(0),
	mOverrideDisplayHeight(0),
	mRejectionScale(0.02f),
	mbDrawCollision(false),
	mCurRenderTargetWidth(0),
	mCurRenderTargetHeight(0),
	mDisplayIsScaled(false),
	mReflectionRenderTargetScale(0.25f),
	mReflectionRejectionScale(0.05f),
	mbShowReflectionMap(false),
	mpReflectionRenderTarget(VUNULL),
	mReflectionMapOffset(0.5f, 0.5f),
	mReflectionMapScale(0.5f, 0.5f),
	mScreenShotSize(0)
{
	memset(&mpRenderTargets, 0, sizeof(mpRenderTargets));

	VuGfxComposer::IF()->setGameInterface(this);

	VuDevMenu::IF()->addBool("GfxComposer/Enable Radial Blur", mRadialBlurEnabled);
	VuDevMenu::IF()->addBool("GfxComposer/Enable Color Correction", mColorCorrectionEnabled);
	VuDevMenu::IF()->addFloat("GfxComposer/Rejection Scale", mRejectionScale, 0.01f, 0, 1);
	VuDevMenu::IF()->addFloat("GfxComposer/Display Scale", mDisplayScale, 0.01f, 0.5f, 1.0f);
	VuDevMenu::IF()->addFloat("GfxComposer/Reflection Render Target Scale", mReflectionRenderTargetScale, 0.01f, 0.1f, 0.5f);
	VuDevMenu::IF()->addFloat("GfxComposer/Reflection Rejection Scale", mReflectionRejectionScale, 0.01f, 0, 1);
	VuDevMenu::IF()->addBool("GfxComposer/Show Reflection Map", mbShowReflectionMap);
	VuDevMenu::IF()->addIntEnum("GfxComposer/Screen Shot Size", mScreenShotSize, sScreenShotChoices);
	VuDevMenu::IF()->addBool("GfxComposer/Draw Collision", mbDrawCollision);

	VuDevConfig::IF()->getParam("DrawCollision").getValue(mbDrawCollision);
}

//*****************************************************************************
VuDefaultGfxComposer::~VuDefaultGfxComposer()
{
	VuGfxComposer::IF()->setGameInterface(VUNULL);
}

//*****************************************************************************
bool VuDefaultGfxComposer::init()
{
	// start rendering
	VuDrawManager::IF()->registerHandler(this, &VuDefaultGfxComposer::draw);

	// add keyboard callback
	VuKeyboard::IF()->addCallback(this);

	return true;
}

//*****************************************************************************
void VuDefaultGfxComposer::release()
{
	// clean up
	VuDrawManager::IF()->unregisterHandler(this);
	VuKeyboard::IF()->removeCallback(this);

	destroyRenderTargets();
}

//*****************************************************************************
VuTexture *VuDefaultGfxComposer::getWaterReflectionTexture(int viewport)
{
	if ( mpReflectionRenderTarget )
		return mpReflectionRenderTarget->getColorTexture();

	return VUNULL;
}

//*****************************************************************************
VuTexture *VuDefaultGfxComposer::getDepthTexture()
{
	return VUNULL;
}

//*****************************************************************************
void VuDefaultGfxComposer::onKeyDown(VUUINT32 key)
{
	if ( key == VUKEY_T )
	{
		takeScreenShot();
	}
}

//*****************************************************************************
void VuDefaultGfxComposer::draw()
{
	// update resources if viewports have changed
	updateRenderTargets();

	VuGfxSort::IF()->setScreen(0);
	VuGfxSort::IF()->setFullScreenLayer(0);
	VuGfxSort::IF()->setViewport(0);
	VuGfxSort::IF()->setReflectionLayer(0);
	VuGfxSort::IF()->setViewportLayer(0);

	VuGfxComposerSceneCommands::submitBeginEndScene(VUNULL);
	if ( VuViewportManager::IF()->getViewportCount() == 0 )
	{
		VuSetRenderTargetParams params(VUNULL);
		params.mColorLoadAction = VuSetRenderTargetParams::LoadActionClear;
		params.mDepthLoadAction = VuSetRenderTargetParams::LoadActionClear;
		VuGfxUtil::IF()->submitSetRenderTargetCommand(params, 1);
	}

	VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_GAME);
	VuGfxSort::IF()->setViewport(0);

	const VuViewportManager::VuViewport &viewport = VuViewportManager::IF()->getViewport(0);
	const VuCamera &camera = viewport.mCamera;

	VUUINT32 zoneMask = Vu3dDrawManager::IF()->calcZoneMask(camera.getEyePosition());

	// determine effects
	bool bRadialBlur = mRadialBlurEnabled && viewport.mRadialBlurAmount > 0.0f;
	bool bLensWater = VuLensWaterManager::IF()->isEnabled() && VuLensWaterManager::IF()->isActive(0);
	bool bEffectsActive = bRadialBlur || bLensWater || mColorCorrectionEnabled || mDisplayIsScaled;

	// handle gfx settings
	VuGfxSettings gfxSettings;
	VuGfxSettingsManager::IF()->getSettings(camera.getEyePosition(), gfxSettings);
	VuGfxSort::IF()->submitGfxSettings(gfxSettings);

	// compose reflection map
	submitReflectionCommands(camera, gfxSettings, zoneMask);

	// submit scene commands
	submitSceneCommands(bEffectsActive ? mpRenderTargets[0] : VUNULL, camera, gfxSettings, zoneMask);

	drawReflectionMap(getWaterReflectionTexture(0));

	// submit ui camera
	VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_HUD);
	VuGfxSort::IF()->setReflectionLayer(0);
	VuGfxSort::IF()->submitCamera(VuViewportManager::IF()->getUiCamera());

	// post process
	if ( mpRenderTargets[0] )
	{
		VuRenderTarget *pCurRenderTarget = mpRenderTargets[0];
		VuRenderTarget *pNextRenderTarget = mpRenderTargets[1];

		// radial blur
		if ( bRadialBlur )
		{
			if ( !bLensWater && !mColorCorrectionEnabled && !mDisplayIsScaled )
				pNextRenderTarget = VUNULL;

			VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_EFFECTS);
			VuGfxSort::IF()->setViewportLayer(0);
			VuGfxComposerPostProcessCommands::radialBlur(pCurRenderTarget->getColorTexture(), pNextRenderTarget, viewport.mRadialBlurAmount);
			VuSwap(pCurRenderTarget, pNextRenderTarget);
		}

		// lens water
		if ( bLensWater )
		{
			if ( !mColorCorrectionEnabled && !mDisplayIsScaled )
				pNextRenderTarget = VUNULL;

			VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_EFFECTS);
			VuGfxSort::IF()->setViewportLayer(1);
			VuLensWaterManager::IF()->submit(0, pCurRenderTarget->getColorTexture(), pNextRenderTarget);
			VuSwap(pCurRenderTarget, pNextRenderTarget);
		}

		// color correction
		if ( mColorCorrectionEnabled )
		{
			if ( !mDisplayIsScaled )
				pNextRenderTarget = VUNULL;

			VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_EFFECTS);
			VuGfxSort::IF()->setViewportLayer(2);
			VuGfxComposerPostProcessCommands::colorCorrection(pCurRenderTarget->getColorTexture(), pNextRenderTarget, gfxSettings.mContrast, gfxSettings.mTint, gfxSettings.mGammaMin, gfxSettings.mGammaMax, gfxSettings.mGammaCurve);
			VuSwap(pCurRenderTarget, pNextRenderTarget);
		}

		// stretch-copy
		if ( mDisplayIsScaled && !mDisablePresent )
		{
			VuGfxSort::IF()->setFullScreenLayer(mScaledUI ? VuGfxSort::FSL_END : VuGfxSort::FSL_EFFECTS);
			VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_END);
			VuGfxComposerPostProcessCommands::copy(pCurRenderTarget->getColorTexture(), VUNULL);
		}
	}
	
	VuGfxSort::IF()->setViewport(0);
}

//*****************************************************************************
void VuDefaultGfxComposer::submitSceneCommands(VuRenderTarget *pRenderTarget, const VuCamera &camera, const VuGfxSettings &gfxSettings, VUUINT32 zoneMask)
{
	VU_PROFILE_SIM("Scene");

	VuGfxSort::IF()->setReflectionLayer(VuGfxSort::REFLECTION_OFF);

	// submit commands
	VuGfxComposerSceneCommands::submitClear(pRenderTarget);

	// draw scene
	VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_WORLD);
	if ( mbDrawCollision )
	{
		VuDynamics::IF()->drawCollision(camera);
	}
	else
	{
		VuGfxDrawParams params(camera);
		params.mRejectionScale = mRejectionScale;
		params.mbDrawReflection = false;
		params.mZoneMask = zoneMask;
		Vu3dDrawManager::IF()->draw(params);
		VuFoliageManager::IF()->draw();
	}

	// submit camera
	VuGfxSort::IF()->submitCamera(camera);
}

//*****************************************************************************
void VuDefaultGfxComposer::submitReflectionCommands(const VuCamera &camera, const VuGfxSettings &gfxSettings, VUUINT32 zoneMask)
{
	if ( VuWater::IF()->getProdecuralReflectionsEnabled() )
	{
		VU_PROFILE_SIM("Reflection");

		VuGfxSort::IF()->setReflectionLayer(VuGfxSort::REFLECTION_ON);

		// calculate water reflection plane
		float waterSurfaceZ = VuViewportManager::IF()->getViewport(0).mCurWaterSurfaceZ;
		VuVector4 reflectionPlane = VuMathUtil::planeFromNormalPoint(VuVector3(0,0,1), VuVector3(0,0,waterSurfaceZ));

		// create reflected camera
		VuCamera reflectionCamera = camera;
		{
			VuVector3 vEye = camera.getEyePosition();
			VuVector3 vTarget = camera.getTargetPosition();
			VuVector3 vUp = camera.getTransform().getAxisZ();

			vEye.mZ = waterSurfaceZ - (vEye.mZ - waterSurfaceZ);
			vTarget.mZ = waterSurfaceZ - (vTarget.mZ - waterSurfaceZ);
			vUp = VuVector3(-vUp.mX, -vUp.mY, vUp.mZ);

			reflectionCamera.setViewMatrix(vEye, vTarget, vUp);
		}

		// transform reflection plane into view space
		VuMatrix mat = reflectionCamera.getViewProjMatrix();
		mat.invert();
		mat.transpose();
		VuVector4 plane = mat.transform(reflectionPlane);

		// submit clip plane commands
		VuGfxComposerSceneCommands::submitReflectionClip(plane);

		// submit scene commands
		VuGfxComposerSceneCommands::submitClear(mpReflectionRenderTarget);

		// draw scene
		VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_WORLD);
		if ( mbDrawCollision )
		{
			VuDynamics::IF()->drawCollision(camera);
		}
		else
		{
			VuGfxDrawParams params(reflectionCamera);
			params.mRejectionScale = mReflectionRejectionScale;
			params.mbDrawReflection = true;
			params.mReflectionPlane = reflectionPlane;
			params.mZoneMask = zoneMask;
			Vu3dDrawManager::IF()->draw(params);
			VuFoliageManager::IF()->draw();
		}

		// submit camera
		VuGfxSort::IF()->submitCamera(reflectionCamera);

		VuGfxSort::IF()->setReflectionLayer(VuGfxSort::REFLECTION_OFF);
	}
}

//*****************************************************************************
void VuDefaultGfxComposer::drawReflectionMap(VuTexture *pReflectionTexture)
{
	if ( mbShowReflectionMap )
	{
		VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_UI);
		
		VuRect rect = VuViewportManager::IF()->getViewport(0).mRect;
		
		float fAspect = rect.mWidth/rect.mHeight;
		
		float fSizeX = 0.2f;
		float fSizeY = fSizeX*fAspect;
		
		float fStartX = 1 - fSizeX;
		float fStartY = 1 - fSizeY;
		
		VuGfxUtil::IF()->drawTexture2d(0, pReflectionTexture, VuRect(fStartX, fStartY, fSizeX, fSizeY));
	}
	else if ( pReflectionTexture )
	{
		// crazy iOS issue: if render target is not used, it is 'lost'
		VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_UI);
		VuGfxUtil::IF()->drawTexture2d(0, pReflectionTexture, VuRect(0,0,0,0));
	}
}

//*****************************************************************************
void VuDefaultGfxComposer::updateRenderTargets()
{
	int displayWidth, displayHeight;
	VuGfx::IF()->getDisplaySize(VUNULL, displayWidth, displayHeight);
	int newWidth = displayWidth;
	int newHeight = displayHeight;

	newWidth = VuRound(newWidth*mDisplayScale);
	newHeight = VuRound(newHeight*mDisplayScale);

	if ( mOverrideDisplayWidth && mOverrideDisplayHeight )
	{
		newWidth = mOverrideDisplayWidth;
		newHeight = mOverrideDisplayHeight;
	}

	// calculate current and new render target counts
	int curRenderTargetCount = 0;
	if ( mpRenderTargets[0] ) curRenderTargetCount++;
	if ( mpRenderTargets[1] ) curRenderTargetCount++;

	int newRenderTargetCount = 0;
	if ( newWidth != displayWidth || newHeight != displayHeight )
		newRenderTargetCount++;
	if ( mRadialBlurEnabled )
		newRenderTargetCount++;
	if ( VuLensWaterManager::IF()->isEnabled() )
		newRenderTargetCount++;
	if ( mColorCorrectionEnabled )
		newRenderTargetCount++;
	newRenderTargetCount = VuMin(newRenderTargetCount, 2);

	// reflections enabled?
	bool curReflectionsEnabled = mpReflectionRenderTarget ? true : false;

	// check if update is needed
	bool bUpdate = false;
	if ( mCurRenderTargetWidth != newWidth || mCurRenderTargetHeight != newHeight )
		bUpdate = true;
	if ( curRenderTargetCount != newRenderTargetCount )
		bUpdate = true;
	if ( curReflectionsEnabled != VuWater::IF()->getProdecuralReflectionsEnabled() )
		bUpdate = true;

	if ( bUpdate )
	{
		destroyRenderTargets();

		if ( newRenderTargetCount >= 1 )
			mpRenderTargets[0] = VuGfx::IF()->createRenderTarget(newWidth, newHeight);
		if ( newRenderTargetCount >= 2 )
			mpRenderTargets[1] = VuGfx::IF()->createRenderTarget(newWidth, newHeight);

		if ( VuWater::IF()->getProdecuralReflectionsEnabled() )
		{
			int refWidth = VuRound(mReflectionRenderTargetScale*newWidth);
			int refHeight = VuRound(mReflectionRenderTargetScale*newHeight);
			mpReflectionRenderTarget = VuGfx::IF()->createRenderTarget(refWidth, refHeight);
		}

		mCurRenderTargetWidth = newWidth;
		mCurRenderTargetHeight = newHeight;

		mDisplayIsScaled = ( newWidth != displayWidth || newHeight != displayHeight );
	}

	// update lens water
	VuLensWaterManager::IF()->setViewportCount(1);
	VuLensWaterManager::IF()->updateTextureSize(0, newWidth >> 1, newHeight >> 1);
}

//*****************************************************************************
void VuDefaultGfxComposer::destroyRenderTargets()
{
	VuGfxSort::IF()->flush();

	for ( int i = 0; i < 2; i++ )
	{
		if ( mpRenderTargets[i] )
		{
			mpRenderTargets[i]->removeRef();
			mpRenderTargets[i] = VUNULL;
		}
	}

	if ( mpReflectionRenderTarget )
	{
		mpReflectionRenderTarget->removeRef();
		mpReflectionRenderTarget = VUNULL;
	}

	mCurRenderTargetWidth = 0;
	mCurRenderTargetHeight = 0;
}

//*****************************************************************************
void VuDefaultGfxComposer::takeScreenShot()
{
	VuGfxSort::IF()->flush();

	// determine screen shot metrics
	ScreenShotMetrics metrics;
	int displayWidth, displayHeight;
	VuGfx::IF()->getDisplaySize(VUNULL, displayWidth, displayHeight);
	metrics.mWidth = displayWidth;
	metrics.mHeight = displayHeight;
	metrics.mRefWidth = VuRound(mReflectionRenderTargetScale*metrics.mWidth);
	metrics.mRefHeight = VuRound(mReflectionRenderTargetScale*metrics.mHeight);
	metrics.mCount = 1;

	if ( mScreenShotSize > 0 )
	{
		metrics = sScreenShotMetrics[mScreenShotSize];
	}

	// create new render target
	destroyRenderTargets();
	VuRenderTarget *pRenderTarget = VuGfx::IF()->createRenderTarget(metrics.mWidth, metrics.mHeight/metrics.mCount);
	mpReflectionRenderTarget = VuGfx::IF()->createRenderTarget(metrics.mRefWidth, metrics.mRefHeight);

	// start screenshot
	VuScreenShotWriter screenShotWriter(metrics.mWidth, metrics.mHeight);

	for ( int iy = 0; iy < metrics.mCount; iy++ )
	{
		VuWater::IF()->renderer()->kick();

		VuGfxSort::IF()->setScreen(0);

		VuGfxComposerSceneCommands::submitBeginEndScene(VUNULL);

		VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_GAME);
		VuGfxSort::IF()->setViewport(0);

		VuCamera camera = VuViewportManager::IF()->getViewport(0).mCamera;
		camera.setProjMatrixVert(camera.getFovVert(), (float)metrics.mWidth/metrics.mHeight, camera.getNearPlane(), camera.getFarPlane());
		VuCamera screenShotCamera = camera;
		screenShotCamera.screenShotShear(0, iy, 1, metrics.mCount);

		VUUINT32 zoneMask = Vu3dDrawManager::IF()->calcZoneMask(camera.getEyePosition());

		mReflectionMapOffset = VuVector2(0.5f, (0.5f + metrics.mCount - iy - 1)/metrics.mCount);
		mReflectionMapScale = VuVector2(0.5f, 0.5f/metrics.mCount);

		// handle gfx settings
		VuGfxSettings gfxSettings;
		VuGfxSettingsManager::IF()->getSettings(camera.getEyePosition(), gfxSettings);
		VuGfxSort::IF()->submitGfxSettings(gfxSettings);

		// compose reflection map
		submitReflectionCommands(camera, gfxSettings, zoneMask);

		// submit scene commands
		submitSceneCommands(pRenderTarget, screenShotCamera, gfxSettings, zoneMask);

		// copy to framebuffer
		VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_END);
		VuGfxComposerPostProcessCommands::copy(pRenderTarget->getColorTexture(), VUNULL);

		VuGfxSort::IF()->draw();
		VuGfxSort::IF()->flush();

		VuArray<VUBYTE> rgb(0);
		pRenderTarget->readPixels(rgb);
		if ( rgb.size() )
		{
			// convert to BGR
			VuImageUtil::swapRB(&rgb[0], metrics.mWidth*metrics.mHeight, 3);
			screenShotWriter.write(&rgb[0], rgb.size());
		}
	}

	// cleanup
	pRenderTarget->removeRef();
	destroyRenderTargets();
	updateRenderTargets();

	mReflectionMapOffset = VuVector2(0.5f, 0.5f);
	mReflectionMapScale = VuVector2(0.5f, 0.5f);
}