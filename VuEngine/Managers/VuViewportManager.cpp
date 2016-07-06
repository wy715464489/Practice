//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ViewportManager class.
// 
//*****************************************************************************

#include "VuViewportManager.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Managers/VuConfigManager.h"
#include "VuEngine/Water/VuWater.h"
#include "VuEngine/Math/VuMathUtil.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuViewportManager, VuViewportManager);

// constants
#define WATER_SURFACE_Z_SNAP_DIST 10.0f	// m
#define WATER_SURFACE_Z_RATE 50.0f		// m/s


//*****************************************************************************
VuViewportManager::VuViewportManager():
	mViewportCount(0),
	mSafeZone(1.0f),
	mSafeZoneRect(0,0,1,1)
{
}

//*****************************************************************************
VuViewportManager::~VuViewportManager()
{
}

//*****************************************************************************
bool VuViewportManager::init()
{
	// register phased tick
	VuTickManager::IF()->registerHandler(this, &VuViewportManager::tickBuild, "Build");

	// register draw
	VuDrawManager::IF()->registerHandler(this, &VuViewportManager::draw);

	mUiCamera.setViewMatrix(VuVector3(0,0,0), VuVector3(0,1,0), VuVector3(0,0,1));

	return true;
}

//*****************************************************************************
void VuViewportManager::release()
{
	// unregister
	VuTickManager::IF()->unregisterHandlers(this);
	VuDrawManager::IF()->unregisterHandler(this);
}

//*****************************************************************************
void VuViewportManager::reset()
{
	mViewportCount = 0;
}

//*****************************************************************************
void VuViewportManager::setViewportCount(int viewportCount)
{
	VUASSERT(viewportCount <= MAX_VIEWPORTS, "VuViewportManager::reset() exceeded max viewport count");
	viewportCount = VuMin(viewportCount, MAX_VIEWPORTS);

	if ( mViewportCount != viewportCount )
	{
		mViewportCount = viewportCount;

		if ( mViewportCount == 1 )
		{
			mViewports[0].mUnsafeRect = VuRect(0.0f, 0.0f, 1.0f, 1.0f);
		}
		else if ( mViewportCount == 2 )
		{
			mViewports[0].mUnsafeRect = VuRect(0.0f, 0.0f, 1.0f, 0.5f);
			mViewports[1].mUnsafeRect = VuRect(0.0f, 0.5f, 1.0f, 0.5f);
		}
		else if ( mViewportCount == 3 )
		{
			mViewports[0].mUnsafeRect = VuRect(0.0f, 0.0f, 1.0f, 0.5f);
			mViewports[1].mUnsafeRect = VuRect(0.0f, 0.5f, 0.5f, 0.5f);
			mViewports[2].mUnsafeRect = VuRect(0.5f, 0.5f, 0.5f, 0.5f);
		}
		else if ( mViewportCount == 4 )
		{
			mViewports[0].mUnsafeRect = VuRect(0.0f, 0.0f, 0.5f, 0.5f);
			mViewports[1].mUnsafeRect = VuRect(0.5f, 0.0f, 0.5f, 0.5f);
			mViewports[2].mUnsafeRect = VuRect(0.0f, 0.5f, 0.5f, 0.5f);
			mViewports[3].mUnsafeRect = VuRect(0.5f, 0.5f, 0.5f, 0.5f);
		}
		else if ( mViewportCount == 5 )
		{
			mViewports[0].mUnsafeRect = VuRect(0.0f, 0.0f, 0.5f, 0.5f);
			mViewports[1].mUnsafeRect = VuRect(0.5f, 0.0f, 0.5f, 0.5f);
			mViewports[2].mUnsafeRect = VuRect(0.0f, 0.5f, 0.3333f, 0.5f);
			mViewports[3].mUnsafeRect = VuRect(0.3333f, 0.5f, 0.3334f, 0.5f);
			mViewports[4].mUnsafeRect = VuRect(0.6667f, 0.5f, 0.3333f, 0.5f);
		}
		else if ( mViewportCount == 6 )
		{
			mViewports[0].mUnsafeRect = VuRect(0.0f, 0.0f, 0.3333f, 0.5f);
			mViewports[1].mUnsafeRect = VuRect(0.3333f, 0.0f, 0.3334f, 0.5f);
			mViewports[2].mUnsafeRect = VuRect(0.6667f, 0.0f, 0.3333f, 0.5f);

			mViewports[3].mUnsafeRect = VuRect(0.0f, 0.5f, 0.3333f, 0.5f);
			mViewports[4].mUnsafeRect = VuRect(0.3333f, 0.5f, 0.3334f, 0.5f);
			mViewports[5].mUnsafeRect = VuRect(0.6667f, 0.5f, 0.3333f, 0.5f);
		}

		applySafeZone();
	}
}

//*****************************************************************************
void VuViewportManager::setCamera(int viewportIndex, const VuCamera &camera)
{
	VUASSERT(viewportIndex >= 0 && viewportIndex < MAX_VIEWPORTS, "VuViewportManager::setCamera() invalid viewport");
	mViewports[viewportIndex].mCamera = camera;
}

//*****************************************************************************
void VuViewportManager::setUiCameraHorz(float fFovHorz, float fAspectRatio, float fNear, float fFar)
{
	mUiCamera.setProjMatrixHorz(fFovHorz, fAspectRatio, fNear, fFar);
}

//*****************************************************************************
void VuViewportManager::setUiCameraVert(float fFovVert, float fAspectRatio, float fNear, float fFar)
{
	mUiCamera.setProjMatrixVert(fFovVert, fAspectRatio, fNear, fFar);
}

//*****************************************************************************
void VuViewportManager::resetEffects()
{
	for ( int viewportIndex = 0; viewportIndex < MAX_VIEWPORTS; viewportIndex++ )
	{
		VuViewport &vp = mViewports[viewportIndex];

		vp.mRadialBlurAmount = 0.0f;
	}
}

//*****************************************************************************
void VuViewportManager::setRadialBlur(int viewportIndex, float amount)
{
	VUASSERT(viewportIndex >= 0 && viewportIndex < mViewportCount, "VuViewportManager::setCamera() invalid viewport");
	VuViewport &vp = mViewports[viewportIndex];

	vp.mRadialBlurAmount = amount;
}

//*****************************************************************************
void VuViewportManager::tickBuild(float fdt)
{
	for ( int viewportIndex = 0; viewportIndex < mViewportCount; viewportIndex++ )
	{
		VuViewport &vp = mViewports[viewportIndex];

		const VuCamera &camera = vp.mCamera;

		// update water reflection z

		// get water reflection z for camera location
		float fWaterSurfaceZ = 0;
		float fWaterSurfaceDist = 0;
		if ( VuWater::IF() )
		{
			VuVector3 extents(camera.getFarPlane(), camera.getFarPlane(), camera.getFarPlane());
			VuAabb aabb(camera.getEyePosition() - extents, camera.getEyePosition() + extents);
			VuWater::IF()->getWaterSurfaceReflectionZ(camera.getEyePosition(), aabb, fWaterSurfaceZ, fWaterSurfaceDist);
		}

		// new water surface detected?
		if ( fWaterSurfaceZ != vp.mDstWaterSurfaceZ )
		{
			vp.mSrcWaterSurfaceZ = vp.mCurWaterSurfaceZ;
			vp.mDstWaterSurfaceZ = fWaterSurfaceZ;
			vp.mOrigWaterSurfaceDist = fWaterSurfaceDist;
		}

		if ( vp.mCurWaterSurfaceZ != vp.mDstWaterSurfaceZ )
		{
			if ( fWaterSurfaceDist < FLT_EPSILON )
			{
				// snap
				vp.mCurWaterSurfaceZ = vp.mDstWaterSurfaceZ;
			}
			else
			{
				// interpolate
				vp.mCurWaterSurfaceZ = VuLerp(vp.mDstWaterSurfaceZ, vp.mSrcWaterSurfaceZ, fWaterSurfaceDist/vp.mOrigWaterSurfaceDist);
			}
		}
	}

	applySafeZone();
}

//*****************************************************************************
void VuViewportManager::draw()
{
	struct DrawData
	{
		static void callback(void *data)
		{
			DrawData *pData = static_cast<DrawData *>(data);

			int displayWidth, displayHeight;
			VuGfx::IF()->getDisplaySize(VUNULL, displayWidth, displayHeight);

			VuGfx::IF()->setViewport(VuRect(0,0,1,1));

			VuGfxUtil::IF()->basicShaders()->set2dXyzConstants(VuMatrix::identity(), VuColor(0,0,0));

			VuVector2 vo0 = VuVector2(0,0);
			VuVector2 vo1 = VuVector2(1,0);
			VuVector2 vo2 = VuVector2(1,1);
			VuVector2 vo3 = VuVector2(0,1);
			VuVector2 vi0 = pData->mSafeZoneRect.getTopLeft();
			VuVector2 vi1 = pData->mSafeZoneRect.getTopRight();
			VuVector2 vi2 = pData->mSafeZoneRect.getBottomRight();
			VuVector2 vi3 = pData->mSafeZoneRect.getBottomLeft();

			VuVertex2dXyz verts[10];
			verts[0].mXyz[0] = vo0.mX; verts[0].mXyz[1] = vo0.mY; verts[0].mXyz[2] = 1.0f;
			verts[1].mXyz[0] = vi0.mX; verts[1].mXyz[1] = vi0.mY; verts[1].mXyz[2] = 1.0f;
			verts[2].mXyz[0] = vo1.mX; verts[2].mXyz[1] = vo1.mY; verts[2].mXyz[2] = 1.0f;
			verts[3].mXyz[0] = vi1.mX; verts[3].mXyz[1] = vi1.mY; verts[3].mXyz[2] = 1.0f;
			verts[4].mXyz[0] = vo2.mX; verts[4].mXyz[1] = vo2.mY; verts[4].mXyz[2] = 1.0f;
			verts[5].mXyz[0] = vi2.mX; verts[5].mXyz[1] = vi2.mY; verts[5].mXyz[2] = 1.0f;
			verts[6].mXyz[0] = vo3.mX; verts[6].mXyz[1] = vo3.mY; verts[6].mXyz[2] = 1.0f;
			verts[7].mXyz[0] = vi3.mX; verts[7].mXyz[1] = vi3.mY; verts[7].mXyz[2] = 1.0f;
			verts[8].mXyz[0] = vo0.mX; verts[8].mXyz[1] = vo0.mY; verts[8].mXyz[2] = 1.0f;
			verts[9].mXyz[0] = vi0.mX; verts[9].mXyz[1] = vi0.mY; verts[9].mXyz[2] = 1.0f;

			VuGfx::IF()->drawPrimitiveUP(VUGFX_PT_TRIANGLESTRIP, 8, verts);

			VuGfx::IF()->setViewport(pData->mSafeZoneRect);
		}

		VuRect	mSafeZoneRect;
	};

	if ( mSafeZone < 1.0f )
	{
		VuGfxSort::IF()->setScreen(0);
		VuGfxSort::IF()->setFullScreenLayer(0);
		VuGfxSort::IF()->setViewport(0);
		VuGfxSort::IF()->setReflectionLayer(0);
		VuGfxSort::IF()->setViewportLayer(0);

		VuGfxUtil::IF()->submitSetViewportCommand(mSafeZoneRect, 1);

		VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_END);
		VuGfxSort::IF()->setViewport(0);
		VuGfxSort::IF()->setReflectionLayer(0);
		VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_END);

		DrawData *pData = static_cast<DrawData *>(VuGfxSort::IF()->allocateCommandMemory(sizeof(DrawData)));
		pData->mSafeZoneRect = mSafeZoneRect;
		VuGfxSort::IF()->submitDrawCommand<true>(VuGfxSort::TRANS_UI_OPAQUE, VuGfxUtil::IF()->basicShaders()->get2dXyzMaterial(VuBasicShaders::FLV_OPAQUE), VUNULL, &DrawData::callback, 1.0f);
	}
}

//*****************************************************************************
void VuViewportManager::applySafeZone()
{
	if ( VuConfigManager::IF() )
	{
		VuConfigManager::Float *pSafeZone = VuConfigManager::IF()->getFloat("Gfx/SafeZone");
		if ( pSafeZone )
			mSafeZone = pSafeZone->mValue;
	}

	int displayWidth = 100, 
		displayHeight = 100;

	if (VuGfx::IF() != VUNULL)
	{
		VuGfx::IF()->getDisplaySize(VUNULL, displayWidth, displayHeight);
	}

	mSafeZoneRect = VuMathUtil::applySafeZone(VuRect(0,0,1,1), mSafeZone);
	finalize(mSafeZoneRect, displayWidth, displayHeight);

	for ( int viewportIndex = 0; viewportIndex < mViewportCount; viewportIndex++ )
	{
		VuViewport &vp = mViewports[viewportIndex];
		vp.mRect = VuMathUtil::applySafeZone(vp.mUnsafeRect, mSafeZone);
		finalize(vp.mRect, displayWidth, displayHeight);
	}
}

//*****************************************************************************
void VuViewportManager::finalize(VuRect &rect, int displayWidth, int displayHeight)
{
	float left = float(VuRound(rect.getLeft()*displayWidth))/displayWidth;
	float right = float(VuRound(rect.getRight()*displayWidth))/displayWidth;
	float top = float(VuRound(rect.getTop()*displayHeight))/displayHeight;
	float bottom = float(VuRound(rect.getBottom()*displayHeight))/displayHeight;

	rect.set(left, top, right - left, bottom - top);
}


//*****************************************************************************
VuViewportManager::VuViewport::VuViewport() :
	mUnsafeRect(0,0,1,1),
	mRect(0,0,1,1),
	mCurWaterSurfaceZ(0), mDstWaterSurfaceZ(0),
	mRadialBlurAmount(0.0f)
{
	mCamera.setProjMatrixHorz(VU_PIDIV2, 1.0f, 1.0f, 2.0f);
	mCamera.setViewMatrix(VuVector3(0,0,0), VuVector3(0,1,0), VuVector3(0,0,1));
}
