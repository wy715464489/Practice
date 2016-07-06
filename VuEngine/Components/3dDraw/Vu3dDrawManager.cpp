//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  3dDrawManager class
// 
//*****************************************************************************

#include "Vu3dDrawManager.h"
#include "VuEngine/Components/3dDraw/Vu3dDrawComponent.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/Shaders/VuBasicShaders.h"
#include "VuEngine/Gfx/Shaders/VuShadowShader.h"
#include "VuEngine/Gfx/Shadow/VuShadowVolume.h"
#include "VuEngine/Gfx/GfxComposer/VuGfxComposer.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuDrawManager.h"
#include "VuEngine/Gfx/Light/VuLightManager.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/Containers/VuDbvt.h"
#include "VuEngine/Dev/VuDevStat.h"
#include "VuEngine/Dev/VuDevMenu.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(Vu3dDrawManager, Vu3dDrawManager);

// ICollide policies
struct Vu3dDrawManager::VuDrawPolicy
{
	VuDrawPolicy(const VuGfxDrawParams &params) : mParams(params), mCount(0) {}
	virtual void			process(const VuDbvtNode *pNode);
	const VuGfxDrawParams	&mParams;
	int						mCount;
};
struct Vu3dDrawManager::VuDrawShadowPolicy
{
	VuDrawShadowPolicy(const VuGfxDrawShadowParams &params) : mParams(params), mCount(0) {}
	virtual void				process(const VuDbvtNode *pNode);
	const VuGfxDrawShadowParams	&mParams;
	int							mCount;
};


static bool sbShowAABBs = false;



//*****************************************************************************
Vu3dDrawManager::Vu3dDrawManager():
	mDrawPassed(0),
	mDrawRejected(0),
	mShadowPassed(0),
	mShadowRejected(0)
{
	if ( VuDevMenu::IF() )
		VuDevMenu::IF()->addBool("3dDrawManager/Show AABBs", sbShowAABBs);

	VuDevStat::IF()->addPage("3dDrawManager", VuRect(50, 10, 40, 80));

	// register phased tick
	VuTickManager::IF()->registerHandler(this, &Vu3dDrawManager::updateDevStats, "Final");

	VuDrawManager::IF()->registerHandler(this, &Vu3dDrawManager::drawPrefetch);

	// create dynamic bounding volume tree
	mpDbvt = new VuDbvt;
}

//*****************************************************************************
Vu3dDrawManager::~Vu3dDrawManager()
{
	VUASSERT(mZoneMasks.size() == 0, "Vu3dDrawManager zone mask leak!");

	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);

	VuDrawManager::IF()->unregisterHandler(this);

	delete mpDbvt;
}

//*****************************************************************************
void Vu3dDrawManager::draw(const VuGfxDrawParams &params)
{
	int planeCount = 0;
	VuVector4 planes[7];

	// gather lights
	VuLightManager::IF()->gatherLights(params);

	// gather gfx composer info
	if ( VuGfxComposer::IF() )
		VuGfxComposer::IF()->gatherInfo(params);

	// add frustum planes
	const VuVector4 *cameraPlanes = params.mCamera.getFrustumPlanes();
	for ( int i = 0; i < 6; i++ )
		planes[planeCount++] = cameraPlanes[i];

	// add reflection plane
	if ( params.mbDrawReflection )
		planes[planeCount++] = params.mReflectionPlane;

	// traverse tree
	VuDrawPolicy policy(params);
	VuDbvt::collideKDOP<256>(mpDbvt->getRoot(), planes, planeCount, policy);

	// update stats
	mDrawPassed += policy.mCount;
	mDrawRejected += mpDbvt->getLeafCount() - policy.mCount;
}

//*****************************************************************************
void Vu3dDrawManager::drawShadow(const VuGfxDrawShadowParams &params)
{
	int planeCount = 0;
	VuVector4 planes[VuShadowClip::MAX_CLIP_PLANES + 1];

	// add shadow clip planes
	const VuVector4 *shadowClipPlanes = params.mCombinedShadowClip.mClipPlanes;
	for ( int i = 0; i < params.mCombinedShadowClip.mClipPlaneCount; i++ )
		planes[planeCount++] = shadowClipPlanes[i];

	// add reflection plane
	if ( params.mbDrawReflection )
		planes[planeCount++] = params.mReflectionPlane;

	// traverse tree
	VuDrawShadowPolicy policy(params);
	VuDbvt::collideKDOP<256>(mpDbvt->getRoot(), planes, planeCount, policy);

	// update stats
	mShadowPassed += policy.mCount;
	mShadowRejected += mpDbvt->getLeafCount() - policy.mCount;
}

//*****************************************************************************
void Vu3dDrawManager::addPrefetchMethod(PrefetchMethod *pMethod)
{
	// prefetch on iOS because graphics drivers are finicky
#if defined (VUIOS) // || defined (VUWIN32)
	mPrefetchQueue.push_back(pMethod);
#endif
}

//*****************************************************************************
void Vu3dDrawManager::removePrefetchMethod(PrefetchMethod *pMethod)
{
	mPrefetchQueue.removeSwap(pMethod);
}

//*****************************************************************************
VUUINT32 Vu3dDrawManager::calcZoneMask(const VuVector3 &pos)
{
	VUUINT32 zoneMask = 0;

	for ( int i = 0; i < mZoneMasks.size(); i++ )
		zoneMask |= mZoneMasks[i]->calcMask(pos);

	if ( zoneMask == 0 )
		zoneMask = (VUUINT32)~0;

	return zoneMask;
}

//*****************************************************************************
void Vu3dDrawManager::add(Vu3dDrawComponent *p3dDrawComponent)
{
	VuDbvtNode *pNode = mpDbvt->insert(p3dDrawComponent, p3dDrawComponent->getAabb());
	p3dDrawComponent->mpDbvtNode = pNode;
}

//*****************************************************************************
void Vu3dDrawManager::remove(Vu3dDrawComponent *p3dDrawComponent)
{
	mpDbvt->remove(p3dDrawComponent->mpDbvtNode);
	p3dDrawComponent->mpDbvtNode = VUNULL;
}

//*****************************************************************************
void Vu3dDrawManager::updateVisibility(Vu3dDrawComponent *p3dDrawComponent)
{
	mpDbvt->update(p3dDrawComponent->mpDbvtNode, p3dDrawComponent->getAabb());
}

//*****************************************************************************
void Vu3dDrawManager::drawPrefetch()
{
	// prefetch
	if ( mPrefetchQueue.size() )
	{
		VuGfxSort::IF()->setFullScreenLayer(VuGfxSort::FSL_GAME);
		VuGfxSort::IF()->setViewport(0);
		VuGfxSort::IF()->setReflectionLayer(0);
		VuGfxSort::IF()->setViewportLayer(VuGfxSort::VPL_WORLD);

		for ( int i = 0; i < mPrefetchQueue.size(); i++ )
			mPrefetchQueue[i]->execute();

		mPrefetchQueue.clear();
	}
}

//*****************************************************************************
void Vu3dDrawManager::updateDevStats(float fdt)
{
	// dev stats
	if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
	{
		if ( pPage->getName() == "3dDrawManager" )
		{
			pPage->clear();
			pPage->printf("Components: %d\n", mpDbvt->getLeafCount());
			pPage->printf("Draw Passed: %d\n", mDrawPassed);
			pPage->printf("Draw Rejected: %d\n", mDrawRejected);
			pPage->printf("Shadow Passed: %d\n", mShadowPassed);
			pPage->printf("Shadow Rejected: %d\n", mShadowRejected);
		}
	}

	// clear stats
	mDrawPassed = 0;
	mDrawRejected = 0;
	mShadowPassed = 0;
	mShadowRejected = 0;
}

//*****************************************************************************
void Vu3dDrawManager::VuDrawPolicy::process(const VuDbvtNode *pNode)
{
	Vu3dDrawComponent *p3dDrawComponent = static_cast<Vu3dDrawComponent *>(pNode->mpData);

	if ( mParams.mZoneMask & p3dDrawComponent->getZoneBits() )
	{
		if ( (mParams.mbDrawReflection & p3dDrawComponent->mbReflecting) == mParams.mbDrawReflection )
		{
			if ( p3dDrawComponent->mpDrawMethod && p3dDrawComponent->mbDrawing )
			{
				p3dDrawComponent->mpDrawMethod->execute(mParams);

				if ( sbShowAABBs )
					VuGfxUtil::IF()->drawAabbLines(VuColor(255,255,0), p3dDrawComponent->mAabb, mParams.mCamera.getViewProjMatrix());
			}
		}
	}

	mCount++;
}

//*****************************************************************************
void Vu3dDrawManager::VuDrawShadowPolicy::process(const VuDbvtNode *pNode)
{
	Vu3dDrawComponent *p3dDrawComponent = static_cast<Vu3dDrawComponent *>(pNode->mpData);

	if ( mParams.mZoneMask & p3dDrawComponent->getZoneBits() )
	{
		if ( (mParams.mbDrawReflection & p3dDrawComponent->mbReflecting) == mParams.mbDrawReflection )
		{
			if ( p3dDrawComponent->mpDrawShadowMethod && p3dDrawComponent->mbShadowing )
			{
				p3dDrawComponent->mpDrawShadowMethod->execute(mParams);
			}
		}
	}

	mCount++;
}
