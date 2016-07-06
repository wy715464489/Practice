//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  WaterDebugDrawer class
// 
//*****************************************************************************

#include "VuWaterDebugDrawer.h"
#include "VuWater.h"
#include "VuWaterWave.h"
#include "VuWaterSurface.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawComponent.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Containers/VuDbrt.h"
#include "VuEngine/Dev/VuDev.h"
#include "VuEngine/Dev/VuDevMenu.h"


// Dbrt policies

//*****************************************************************************
class VuDrawBoundsPolicy
{
public:
	void process(const VuDbrtNode *pNode)
	{
		// draw outline
		VuRect rect(pNode->mBounds.mMin, pNode->mBounds.mMax);
		VuGfxUtil::IF()->drawRectangleOutline2d(GFX_SORT_DEPTH_STEP, VuColor(64,64,255), rect);
	}
};

//*****************************************************************************
class VuDrawSurfacesPolicy
{
public:
	void process(const VuDbrtNode *pNode)
	{
		// debug draw wave
		VuWaterSurface *pSurface = static_cast<VuWaterSurface *>(pNode->mpData);

		const VuDbrt *pDbrt = pSurface->mpWaveDbrt;
		if ( pDbrt->getRoot() )
		{
			// draw nodes (bounds)
			VuDrawBoundsPolicy drawBoundsPolicy;
			pDbrt->enumNodes(pDbrt->getRoot(), drawBoundsPolicy);
		}
	}
};

//*****************************************************************************
VuWaterDebugDrawer::VuWaterDebugDrawer():
	mbDraw3d(false),
	mbDraw2d(false)
{
	addComponent(mp3dDrawComponent = new Vu3dDrawComponent(this));
	mp3dDrawComponent->setDrawMethod(this, &VuWaterDebugDrawer::draw3d);
	mp3dDrawComponent->updateVisibility(VuAabb(VuVector3(-1e9, -1e9, -1e9), VuVector3(1e9, 1e9, 1e9)));

	VuDevMenu::IF()->addBool("Water/Debug Waves 3d", mbDraw3d);
	VuDevMenu::IF()->addBool("Water/Debug Waves 2d", mbDraw2d);

	// start drawing 3d
	mp3dDrawComponent->show();

	// start drawing 2d
	VuDrawManager::IF()->registerHandler(this, &VuWaterDebugDrawer::draw2d);
}

//*****************************************************************************
VuWaterDebugDrawer::~VuWaterDebugDrawer()
{
	// stop drawing 3d
	mp3dDrawComponent->hide();

	// stop drawing 2d
	VuDrawManager::IF()->unregisterHandler(this);
}

//*****************************************************************************
void VuWaterDebugDrawer::draw3d(const VuGfxDrawParams &params)
{
	if ( !mbDraw3d )
		return;

	// debug draw waves
	for ( VuWater::Waves::Node *pNode = VuWater::IF()->getWaves().mpHead; pNode; pNode = pNode->mpNext )
		pNode->mpValue->debugDraw3d(params.mCamera);
}

//*****************************************************************************
void VuWaterDebugDrawer::draw2d()
{
	if ( !mbDraw2d )
		return;

	VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_WATER_DEBUG);

	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

	// draw background
	pGfxUtil->drawFilledRectangle2d(2*GFX_SORT_DEPTH_STEP, VuColor(0, 0, 0, 128));
	
	// get dynamic bounding rectangle tree root
	const VuDbrt *pDbrt = VuWater::IF()->getSurfaceDbrt();
	if ( !pDbrt->getRoot() )
		return;

	// get bounds
	VuVector2 vMin = pDbrt->getRoot()->mBounds.mMin;
	VuVector2 vMax = pDbrt->getRoot()->mBounds.mMax;

	// calculate aspect ratio
	int displayWidth, displayHeight;
	VuGfx::IF()->getDisplaySize(VUNULL, displayWidth, displayHeight);
	float displayAR = (float)displayWidth/displayHeight; 

	// square it up and apply aspect ratio
	VuVector2 vSize = vMax - vMin;
	VuVector2 vCenter = 0.5f*(vMin + vMax);
	if ( vSize.mX/vSize.mY > displayAR )
		vSize.mY = vSize.mX/displayAR;
	else
		vSize.mX = vSize.mY*displayAR;
	vMin = vCenter - 0.5f*vSize;
	vMax = vCenter + 0.5f*vSize;

	// set up gfxmatrix
	VuMatrix mat;
	mat.loadIdentity();
	mat.translate(VuVector3(-vMin.mX, -vMin.mY, 0));
	mat.scale(VuVector3(1/vSize.mX, 1/vSize.mY, 1));
	mat.scale(VuVector3( 1, -1, 1));
	mat.translate(VuVector3(0, 1, 0));

	pGfxUtil->pushMatrix(mat);
	{
		// draw nodes (bounds)
		VuDrawBoundsPolicy drawBoundsPolicy;
		pDbrt->enumNodes(pDbrt->getRoot(), drawBoundsPolicy);

		// draw leaves (surfaces)
		VuDrawSurfacesPolicy drawSurfacesPolicy;
		pDbrt->enumLeaves(pDbrt->getRoot(), drawSurfacesPolicy);

		// draw waves
		for ( VuWater::Waves::Node *pNode = VuWater::IF()->getWaves().mpHead; pNode; pNode = pNode->mpNext )
			pNode->mpValue->debugDraw2d();
	}
	pGfxUtil->popMatrix();
}


//*****************************************************************************
class VuDrawWavesPolicy
{
public:
	void process(const VuDbrtNode *pNode)
	{
		// debug draw wave
		VuWaterWave *pWave = static_cast<VuWaterWave *>(pNode->mpData);
		pWave->debugDraw2d();
	}
};

