//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water class
// 
//*****************************************************************************

#include "VuWater.h"
#include "VuWaterDebugDrawer.h"
#include "VuWaterSurface.h"
#include "VuWaterShader.h"
#include "VuWaterRenderer.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuConfigManager.h"
#include "VuEngine/Dynamics/VuDynamics.h"
#include "VuEngine/Containers/VuDbrt.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Dev/VuDevStat.h"
#include "VuEngine/Dev/VuDevProfile.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuWater, VuWater);


// constants
#define FREE_POINTER_LIST_SIZE	1024
#define MAX_WATER_VERTS_PER_GET 1024


// static functions
static bool SurfaceComp(const VuWaterSurface *p1, const VuWaterSurface *p2)
{
	return p1->mDesc.mPos.mZ < p2->mDesc.mPos.mZ;
}

//*****************************************************************************
inline bool SurfaceIntersects(const VuWaterSurface *pSurface, const VuVector3 &vPos, float fRadius = 0)
{
	VuVector3 vLocalPos = pSurface->mInverseTransform.transform(vPos);

	if ( VuAbs(vLocalPos.mX) <= pSurface->mExtent.mX + fRadius && VuAbs(vLocalPos.mY) <= pSurface->mExtent.mY + fRadius )
		if ( vLocalPos.mZ <= pSurface->mDesc.mMaxWaveHeight + fRadius && vLocalPos.mZ >= -(pSurface->mDesc.mMaxWaveDepth + fRadius) )
			return true;

	return false;
}

//*****************************************************************************
inline bool SurfaceIntersectsIgnoreHeight(const VuWaterSurface *pSurface, const VuVector3 &vPos, float fRadius = 0)
{
	VuVector3 vLocalPos = pSurface->mInverseTransform.transform(vPos);

	if ( VuAbs(vLocalPos.mX) <= pSurface->mExtent.mX + fRadius && VuAbs(vLocalPos.mY) <= pSurface->mExtent.mY + fRadius )
		if ( vLocalPos.mZ >= -(pSurface->mDesc.mMaxWaveDepth + fRadius) )
			return true;

	return false;
}

//*****************************************************************************
inline bool SurfaceContains(const VuWaterSurface *pSurface, const VuWaterSurfaceDataParams &params)
{
	VuVector3 vLocalPos = pSurface->mInverseTransform.transform(params.mBoundingCenter);
	float fRadius = params.mBoundingRadius;

	if ( VuAbs(vLocalPos.mX) + fRadius <= pSurface->mExtent.mX && VuAbs(vLocalPos.mY) + fRadius <= pSurface->mExtent.mY )
		if ( params.mBoundingAabb.mMax.mZ >= pSurface->mWorldAabb.mMin.mZ )
			return true;

	return false;
}

// Dbrt policies

//*****************************************************************************
class VuBinWavePolicy
{
public:
	VuBinWavePolicy(VuWaterWave *pWave) : mpWave(pWave) {}
	void process(const VuDbrtNode *pNode)
	{
		VuWaterSurface *pSurface = static_cast<VuWaterSurface *>(pNode->mpData);
		VuWater::IF()->maybeAddWaveToSurface(mpWave, pSurface);
	}
	VuWaterWave	*mpWave;
};

//*****************************************************************************
class VuGetWaveDataPolicy
{
public:
	VuGetWaveDataPolicy(VuWaterSurfaceDataParams &params) : mParams(params) {}
	void process(const VuDbrtNode *pNode);
	VuWaterSurfaceDataParams	&mParams;
};

//*****************************************************************************
class VuGetSurfaceForPointPolicy
{
public:
	VuGetSurfaceForPointPolicy(const VuVector3 &vPos) : mPos(vPos), mpResult(VUNULL) {}
	void process(const VuDbrtNode *pNode)
	{
		const VuWaterSurface *pSurface = static_cast<const VuWaterSurface *>(pNode->mpData);
		if ( SurfaceIntersects(pSurface, mPos) )
			mpResult = pSurface;
	}
	VuVector3				mPos;
	const VuWaterSurface	*mpResult;
};

//*****************************************************************************
class VuGetSurfaceForPointIgnoreHeightPolicy
{
public:
	VuGetSurfaceForPointIgnoreHeightPolicy(const VuVector3 &vPos) : mPos(vPos), mSurfaceZ(MIN_WORLD_WATER_HEIGHT), mpResult(VUNULL) {}
	void process(const VuDbrtNode *pNode)
	{
		const VuWaterSurface *pSurface = static_cast<const VuWaterSurface *>(pNode->mpData);
		if ( SurfaceIntersectsIgnoreHeight(pSurface, mPos) )
		{
			if ( pSurface->mDesc.mPos.mZ > mSurfaceZ )
				mpResult = pSurface;
		}
	}
	VuVector3				mPos;
	float					mSurfaceZ;
	const VuWaterSurface	*mpResult;
};

//*****************************************************************************
class VuGetSurfacesForParamsPolicy
{
public:
	enum { MAX_COUNT = 8 };
	VuGetSurfacesForParamsPolicy(const VuWaterSurfaceDataParams &params) : mParams(params), mCount(0) {}
	void process(const VuDbrtNode *pNode)
	{
		const VuWaterSurface *pSurface = static_cast<const VuWaterSurface *>(pNode->mpData);
		if ( SurfaceIntersects(pSurface, mParams.mBoundingCenter, mParams.mBoundingRadius) )
			if ( mCount < MAX_COUNT )
				mpSurfaces[mCount++] = pSurface;
	}
	const VuWaterSurfaceDataParams	&mParams;
	int								mCount;
	const VuWaterSurface			*mpSurfaces[MAX_COUNT];
};

//*****************************************************************************
class VuGetClosestSurfaceForPointPolicy
{
public:
	VuGetClosestSurfaceForPointPolicy(const VuVector3 &vPos) : mPos(vPos), mMinDistSquared(FLT_MAX), mpResult(VUNULL) {}
	void process(const VuDbrtNode *pNode)
	{
		const VuWaterSurface *pSurface = static_cast<const VuWaterSurface *>(pNode->mpData);
		float distSquared = pSurface->calcDistance3dSquared(mPos);
		if ( distSquared < mMinDistSquared )
		{
			mpResult = pSurface;
			mMinDistSquared = distSquared;
		}
	}
	VuVector3				mPos;
	float					mMinDistSquared;
	const VuWaterSurface	*mpResult;
};

//*****************************************************************************
class VuEnumWakeWavesPolicy
{
public:
	VuEnumWakeWavesPolicy(const VuVector3 &vPos, VuWater::VuWakeWaveEnumCB &cb) : mPos(vPos), mCallback(cb) {}
	void process(const VuDbrtNode *pNode);
	VuVector3					mPos;
	VuWater::VuWakeWaveEnumCB	&mCallback;
};


//*****************************************************************************
VuWater::VuWater():
	mPointerFreeList(FREE_POINTER_LIST_SIZE),
	mpDebugDrawer(VUNULL),
	mbDisableWaves(false),
	mDetail(1.0f),
	mProceduralReflectionsEnabled(true),
	mWakesEnabled(true),
	mNormalMapEnabled(true)
{
}

//*****************************************************************************
bool VuWater::init(bool bAsynchronous)
{
	mpRenderer = new VuWaterRenderer(bAsynchronous);

	// set up dev menu/stats
	VuDevMenu::IF()->addBool("Water/Disable Waves", mbDisableWaves);
	VuDevStat::IF()->addPage("Water", VuRect(50, 10, 40, 80));

	// register phased tick
	VuTickManager::IF()->registerHandler(this, &VuWater::tickWater, "Water");
	VuTickManager::IF()->registerHandler(this, &VuWater::updateDevStats, "Final");

	// create water surface dynamic bounding rectangle tree
	mpSurfaceDbrt = new VuDbrt(128, 64);

	// create debug drawer
	mpDebugDrawer = new VuWaterDebugDrawer;

	// configuration
	mDetail = VuConfigManager::IF()->getFloat("Water/Detail")->mValue;
	mProceduralReflectionsEnabled = VuConfigManager::IF()->getBool("Water/Reflection")->mValue;
	mWakesEnabled = VuConfigManager::IF()->getBool("Water/Wakes")->mValue;
	mNormalMapEnabled = VuConfigManager::IF()->getBool("Water/NormalMap")->mValue;

	VuConfigManager::IF()->registerFloatHandler("Water/Detail", this, &VuWater::configDetail);
	VuConfigManager::IF()->registerBoolHandler("Water/Reflection", this, &VuWater::configReflection);
	VuConfigManager::IF()->registerBoolHandler("Water/Wakes", this, &VuWater::configWakes);
	VuConfigManager::IF()->registerBoolHandler("Water/NormalMap", this, &VuWater::configNormalMap);

	return true;
}

//*****************************************************************************
void VuWater::release()
{
	VuConfigManager::IF()->unregisterFloatHandler("Water/Detail", this);
	VuConfigManager::IF()->unregisterBoolHandler("Water/Reflection", this);
	VuConfigManager::IF()->unregisterBoolHandler("Water/Wakes", this);
	VuConfigManager::IF()->unregisterBoolHandler("Water/NormalMap", this);

	VUASSERT(mSurfaces.size() == 0, "VuWater::release() water surface leak"); 

	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);

	// destroy waves
	while ( mWaves.mpHead )
		removeWave(mWaves.mpHead->mpValue);

	// destroy debug drawer
	mpDebugDrawer->removeRef();
	mpDebugDrawer = VUNULL;

	delete mpRenderer;

	delete mpSurfaceDbrt;
}

//*****************************************************************************
VuWaterRenderer *VuWater::renderer()
{
	return mpRenderer;
}

//*****************************************************************************
VuWaterSurface *VuWater::createSurface(const VuWaterSurfaceDesc &desc, VuEntity *pOwnerEntity)
{
	if ( mpRenderer->isBusy() )
	{
		mpRenderer->flush();
		VUPRINTF("VuWater::unbinWave() while water renderer thread is running\n");
	}

	VuWaterSurface *pSurface = new VuWaterSurface(desc, pOwnerEntity);
	mSurfaces.push_back(pSurface);
	mSurfaces.sort(SurfaceComp);

	binSurface(pSurface);

	for ( SurfaceCallbacks::iterator iter = mSurfaceCallbacks.begin(); iter != mSurfaceCallbacks.end(); iter++ )
		(*iter)->onWaterSurfaceCreated(pSurface);

	return pSurface;
}

//*****************************************************************************
VuWaterShader *VuWater::createShader(const VuWaterShaderDesc &desc)
{
	for ( Shaders::iterator iter = mShaders.begin(); iter != mShaders.end(); iter++ )
	{
		if ( (*iter)->getDesc() == desc )
		{
			(*iter)->addRef();
			return *iter;
		}
	}
	
	VuWaterShader *pShader = new VuWaterShader(desc);
	mShaders.push_back(pShader);

	return pShader;
}

//*****************************************************************************
VuWaterWakeWave *VuWater::createWakeWave(const VuWaterWakeWaveDesc &desc, const VuWaterWakeWaveParams &params)
{
	VuWaterWakeWave *pWave = new VuWaterWakeWave(desc, params);
	addCustomWave(pWave);
	return pWave;
}

//*****************************************************************************
VuWaterFlatWakeWave *VuWater::createFlatWakeWave(const VuWaterWakeWaveDesc &desc, const VuWaterWakeWaveParams &params)
{
	VuWaterFlatWakeWave *pWave = new VuWaterFlatWakeWave(desc, params);
	addCustomWave(pWave);
	return pWave;
}

//*****************************************************************************
VuWaterDirectionalWave *VuWater::createDirectionalWave(const VuWaterDirectionalWaveDesc &desc)
{
	VuWaterDirectionalWave *pWave = new VuWaterDirectionalWave(desc);
	addCustomWave(pWave);
	return pWave;
}

//*****************************************************************************
VuWaterDirectionalFlowWave *VuWater::createDirectionalFlowWave(const VuWaterDirectionalFlowWaveDesc &desc)
{
	VuWaterDirectionalFlowWave *pWave = new VuWaterDirectionalFlowWave(desc);
	addCustomWave(pWave);
	return pWave;
}

//*****************************************************************************
VuWaterInfiniteOceanWave *VuWater::createInfiniteOceanWave(const VuWaterInfiniteOceanWaveDesc &desc)
{
	VuWaterInfiniteOceanWave *pWave = new VuWaterInfiniteOceanWave(desc);
	addCustomWave(pWave);
	return pWave;
}

//*****************************************************************************
VuWaterRectangularOceanWave *VuWater::createRectangularOceanWave(const VuWaterRectangularOceanWaveDesc &desc)
{
	VuWaterRectangularOceanWave *pWave = new VuWaterRectangularOceanWave(desc);
	addCustomWave(pWave);
	return pWave;
}

//*****************************************************************************
VuWaterCircularOceanWave *VuWater::createCircularOceanWave(const VuWaterCircularOceanWaveDesc &desc)
{
	VuWaterCircularOceanWave *pWave = new VuWaterCircularOceanWave(desc);
	addCustomWave(pWave);
	return pWave;
}

//*****************************************************************************
VuWaterWhirlpoolWave *VuWater::createWhirlpoolWave(const VuWaterWhirlpoolWaveDesc &desc)
{
	VuWaterWhirlpoolWave *pWave = new VuWaterWhirlpoolWave(desc);
	addCustomWave(pWave);
	return pWave;
}

//*****************************************************************************
VuWaterRampWave *VuWater::createRampWave(const VuWaterRampWaveDesc &desc)
{
	VuWaterRampWave *pWave = new VuWaterRampWave(desc);
	addCustomWave(pWave);
	return pWave;
}

//*****************************************************************************
VuWaterPointWave *VuWater::createPointWave(const VuWaterPointWaveDesc &desc)
{
	VuWaterPointWave *pWave = new VuWaterPointWave(desc);
	addCustomWave(pWave);
	return pWave;
}

//*****************************************************************************
VuWaterBumpWave *VuWater::createBumpWave(const VuWaterBumpWaveDesc &desc)
{
	VuWaterBumpWave *pWave = new VuWaterBumpWave(desc);
	addCustomWave(pWave);
	return pWave;
}

//*****************************************************************************
VuWaterBankedTurnWave *VuWater::createBankedTurnWave(const VuWaterBankedTurnWaveDesc &desc)
{
	VuWaterBankedTurnWave *pWave = new VuWaterBankedTurnWave(desc);
	addCustomWave(pWave);
	return pWave;
}

//*****************************************************************************
void VuWater::addCustomWave(VuWaterWave *pWave)
{
	pWave->addRef();
	mWaves.add(pWave, mPointerFreeList);

	binWave(pWave);
}

//*****************************************************************************
void VuWater::getSurfaceData(VuWaterSurfaceDataParams &params)
{
	VuDbrtBounds bounds(params.mBoundingAabb);

	// check for single surface hint
	if ( params.mpWaterSurfaceHint )
	{
		const VuWaterSurface *pSurface = static_cast<const VuWaterSurface *>(params.mpWaterSurfaceHint);
		initializeOutput(params, pSurface->mDesc.mPos.mZ);
		if ( !mbDisableWaves )
		{
			VuGetWaveDataPolicy policy(params);
			pSurface->mpWaveDbrt->collideBounds(pSurface->mpWaveDbrt->getRoot(), bounds, policy);
		}
		return;
	}

	// get surfaces affecting params
	VuGetSurfacesForParamsPolicy surfacesPolicy(params);
	mpSurfaceDbrt->collideBounds(mpSurfaceDbrt->getRoot(), bounds, surfacesPolicy);
	if ( !surfacesPolicy.mCount )
	{
		initializeOutput(params, MIN_WORLD_WATER_HEIGHT);
		return;
	}

	// check for single surface
	if ( surfacesPolicy.mCount == 1 )
	{
		const VuWaterSurface *pSurface = surfacesPolicy.mpSurfaces[0];
		if ( SurfaceContains(pSurface, params) )
		{
			initializeOutput(params, pSurface->mDesc.mPos.mZ);
			if ( !mbDisableWaves )
			{
				VuGetWaveDataPolicy policy(params);
				pSurface->mpWaveDbrt->collideBounds(pSurface->mpWaveDbrt->getRoot(), bounds, policy);
			}
			return;
		}
	}

	// partial or multiple surfaces
	initializeClipOutput(params, surfacesPolicy.mpSurfaces, surfacesPolicy.mCount);
	if ( !mbDisableWaves )
	{
		// apply surfaces
		for ( int i = 0; i < surfacesPolicy.mCount; i++ )
		{
			const VuWaterSurface *pSurface = surfacesPolicy.mpSurfaces[i];
			VuGetWaveDataPolicy policy(params);
			pSurface->mpWaveDbrt->collideBounds(pSurface->mpWaveDbrt->getRoot(), bounds, policy);
		}
	}
}

//*****************************************************************************
const VuWaterSurface *VuWater::getSurface(const VuVector3 &vPos, bool bIgnoreHeight)
{
	const VuWaterSurface *pSurface = VUNULL;
	if ( bIgnoreHeight )
	{
		VuGetSurfaceForPointIgnoreHeightPolicy policy(vPos);
		mpSurfaceDbrt->collidePoint(mpSurfaceDbrt->getRoot(), VuVector2(vPos.mX, vPos.mY), policy);
		pSurface = policy.mpResult;
	}
	else
	{
		VuGetSurfaceForPointPolicy policy(vPos);
		mpSurfaceDbrt->collidePoint(mpSurfaceDbrt->getRoot(), VuVector2(vPos.mX, vPos.mY), policy);
		pSurface = policy.mpResult;
	}

	return pSurface;
}

//*****************************************************************************
VuWaterPhysicsVertex VuWater::getPhysicsVertex(const VuVector3 &vPosition)
{
	VuWaterPhysicsVertex vert;
	vert.mPosition = vPosition;
	vert.mDxyzDt = VuVector3(0,0,0);
	vert.mHeight = MIN_WORLD_WATER_HEIGHT;

	if ( const VuWaterSurface *pSurface = getSurface(vPosition) )
	{
		vert.mHeight = pSurface->mDesc.mPos.mZ;

		if ( !mbDisableWaves )
		{
			VuWaterSurfaceDataParams params(VuWaterSurfaceDataParams::VT_PHYSICS);

			params.mVertCount = 1;
			params.mBoundingAabb.mMin = vPosition;
			params.mBoundingAabb.mMax = vPosition;
			params.mBoundingCenter = vPosition;
			params.mBoundingRadius = 0;
			params.mpPhysicsVertex = &vert;

			VuGetWaveDataPolicy policy(params);
			pSurface->mpWaveDbrt->collidePoint(pSurface->mpWaveDbrt->getRoot(), VuVector2(vPosition.mX, vPosition.mY), policy);
		}
	}

	return vert;
}

//*****************************************************************************
VuWaterRenderVertex VuWater::getRenderVertex(const VuVector3 &vPosition)
{
	VuWaterRenderVertex vert;
	vert.mPosition = VuPackedVector3(vPosition.mX, vPosition.mY, MIN_WORLD_WATER_HEIGHT);
	vert.mDzDxy = VuPackedVector2(0,0);
	vert.mFoam = 0.0f;

	if ( const VuWaterSurface *pSurface = getSurface(vPosition) )
	{
		vert.mPosition.mZ = pSurface->mDesc.mPos.mZ;

		if ( !mbDisableWaves )
		{
			VuWaterSurfaceDataParams params(VuWaterSurfaceDataParams::VT_RENDER);

			params.mVertCount = 1;
			params.mBoundingAabb.mMin = vPosition;
			params.mBoundingAabb.mMax = vPosition;
			params.mBoundingCenter = vPosition;
			params.mBoundingRadius = 0;
			params.mpRenderVertex = &vert;

			VuGetWaveDataPolicy policy(params);
			pSurface->mpWaveDbrt->collidePoint(pSurface->mpWaveDbrt->getRoot(), VuVector2(vPosition.mX, vPosition.mY), policy);
		}
	}

	return vert;
}

//*****************************************************************************
float VuWater::getWaterSurfaceZ(const VuVector3 &vPosition, const VuAabb &aabb)
{
	VuGetClosestSurfaceForPointPolicy policy(vPosition);
	mpSurfaceDbrt->collideBounds(mpSurfaceDbrt->getRoot(), VuDbrtBounds(aabb), policy);

	if ( policy.mpResult )
		return policy.mpResult->mDesc.mPos.mZ;

	return MIN_WORLD_WATER_HEIGHT;
}

//*****************************************************************************
void VuWater::getWaterSurfaceReflectionZ(const VuVector3 &vPosition, const VuAabb &aabb, float &z, float &dist)
{
	float minDistSquared = FLT_MAX;
	z = 0;

	// step through all water surfaces
	for ( Surfaces::const_iterator iter = mSurfaces.begin(); iter != mSurfaces.end(); iter++ )
	{
		const VuWaterSurface *pSurface = *iter;
		if ( pSurface->mDesc.mProceduralReflection && vPosition.mZ > pSurface->mWorldAabb.mMin.mZ )
		{
			float distSquared = pSurface->calcReflectionDistance3dSquared(vPosition);
			if ( distSquared < minDistSquared )
			{
				z = pSurface->mDesc.mPos.mZ;
				minDistSquared = distSquared;
			}
		}
	}

	dist = VuSqrt(minDistSquared);
}

//*****************************************************************************
bool VuWater::getWaterSurfaceMinZ(float &minZ)
{
	if ( mSurfaces.size() )
	{
		minZ = (*mSurfaces.begin())->mWorldAabb.mMin.mZ;
		return true;
	}

	return false;
}

//*****************************************************************************
void VuWater::enumWakeWaves(const VuVector3 &vPosition, VuWakeWaveEnumCB &cb)
{
	if ( !mbDisableWaves )
	{
		if ( const VuWaterSurface *pSurface = getSurface(vPosition) )
		{
			VuEnumWakeWavesPolicy policy(vPosition, cb);
			pSurface->mpWaveDbrt->collidePoint(pSurface->mpWaveDbrt->getRoot(), VuVector2(vPosition.mX, vPosition.mY), policy);
		}
	}
}

//*****************************************************************************
void VuWater::removeSurface(VuWaterSurface *pSurface)
{
	if ( mpRenderer->isBusy() )
	{
		mpRenderer->flush();
		VUPRINTF("VuWater::unbinWave() while water renderer thread is running\n");
	}

	mSurfaces.remove(pSurface);

	unbinSurface(pSurface);

	for ( SurfaceCallbacks::iterator iter = mSurfaceCallbacks.begin(); iter != mSurfaceCallbacks.end(); iter++ )
		(*iter)->onWaterSurfaceDestroyed(pSurface);
}

//*****************************************************************************
void VuWater::removeShader(VuWaterShader *pShader)
{
	mShaders.remove(pShader);
}

//*****************************************************************************
void VuWater::removeWave(VuWaterWave *pWave)
{
	if ( mWaves.remove(pWave, mPointerFreeList) )
	{
		unbinWave(pWave);
		pWave->removeRef();
	}
}

//*****************************************************************************
void VuWater::binSurface(VuWaterSurface *pSurface)
{
	// add to surface DBRT
	VuDbrtBounds surfaceBounds;
	surfaceBounds.mMin = VuVector2(pSurface->mWorldAabb.mMin.mX, pSurface->mWorldAabb.mMin.mY);
	surfaceBounds.mMax = VuVector2(pSurface->mWorldAabb.mMax.mX, pSurface->mWorldAabb.mMax.mY);
	VuDbrtNode *pNode = mpSurfaceDbrt->insert(pSurface, surfaceBounds);
	pSurface->mpDbrtNode = pNode;

	// add waves to this surface
	for ( Waves::Node *pWaveNode = mWaves.mpHead; pWaveNode; pWaveNode = pWaveNode->mpNext )
	{
		VuWaterWave *pWave = pWaveNode->mpValue;

		if ( pWave->mFlags & VuWaterWave::POINT_SURFACE_BINNING )
		{
			// check point
			VuVector3 point = pWave->mBoundingAabb.getCenter();
			if ( surfaceBounds.contains(VuVector2(point.mX, point.mY)) )
				maybeAddWaveToSurface(pWave, pSurface);
		}
		else
		{
			// check bounds
			VuDbrtBounds waveBounds;
			waveBounds.mMin = VuVector2(pWave->mBoundingAabb.mMin.mX, pWave->mBoundingAabb.mMin.mY);
			waveBounds.mMax = VuVector2(pWave->mBoundingAabb.mMax.mX, pWave->mBoundingAabb.mMax.mY);

			if ( surfaceBounds.intersects(waveBounds) )
				maybeAddWaveToSurface(pWave, pSurface);
		}
	}
}

//*****************************************************************************
void VuWater::unbinSurface(VuWaterSurface *pSurface)
{
	// remove from surface DBRT
	mpSurfaceDbrt->remove(pSurface->mpDbrtNode);
	pSurface->mpDbrtNode = VUNULL;

	// clear the wave DBRT
	pSurface->mpWaveDbrt->clear();

	// remove any wave DBRT references to this surface
	for ( Waves::Node *pWaveNode = mWaves.mpHead; pWaveNode; pWaveNode = pWaveNode->mpNext )
	{
		VuWaterWave::DbrtNodes::Node *pDbrtNode = pWaveNode->mpValue->mDbrtNodes.mpHead;
		while ( pDbrtNode )
		{
			VuWaterWave::DbrtNodes::Node *pNextDbrtNode = pDbrtNode->mpNext;
			if ( pDbrtNode->mpValue->mpExtraData == pSurface )
				pWaveNode->mpValue->mDbrtNodes.remove(pDbrtNode->mpValue, mPointerFreeList);
			pDbrtNode = pNextDbrtNode;
		}
	}
}

//*****************************************************************************
void VuWater::rebinSurface(VuWaterSurface *pSurface)
{
	if ( pSurface->mpDbrtNode )
	{
		unbinSurface(pSurface);
		binSurface(pSurface);
	}
}

//*****************************************************************************
void VuWater::binWave(VuWaterWave *pWave)
{
	if ( VuDynamics::IF() && VuDynamics::IF()->isBusy() )
	{
		VuDynamics::IF()->flush();
		VUWARNING("VuWater::binWave() while dynamics thread is running");
	}

	if ( mpRenderer->isBusy() )
	{
		mpRenderer->flush();
		VUWARNING("VuWater::unbinWave() while water renderer thread is running");
	}

	if ( pWave->mFlags & VuWaterWave::POINT_SURFACE_BINNING )
	{
		// add wave to single water surface
		VuBinWavePolicy policy(pWave);
		VuVector3 point = pWave->mBoundingAabb.getCenter();
		mpSurfaceDbrt->collidePoint(mpSurfaceDbrt->getRoot(), VuVector2(point.mX, point.mY), policy);
	}
	else
	{
		// add wave to water surfaces
		VuBinWavePolicy policy(pWave);
		mpSurfaceDbrt->collideBounds(mpSurfaceDbrt->getRoot(), VuDbrtBounds(pWave->mBoundingAabb), policy);
	}
}

//*****************************************************************************
void VuWater::unbinWave(VuWaterWave *pWave)
{
	if ( VuDynamics::IF() && VuDynamics::IF()->isBusy() )
	{
		VuDynamics::IF()->flush();
		VUPRINTF("VuWater::unbinWave() while dynamics thread is running\n");
	}

	if ( mpRenderer->isBusy() )
	{
		mpRenderer->flush();
		VUPRINTF("VuWater::unbinWave() while water renderer thread is running\n");
	}

	// remove wave from water surfaces
	while ( pWave->mDbrtNodes.mpHead )
	{
		VuDbrtNode *pWaveNode = pWave->mDbrtNodes.mpHead->mpValue;
		VuWaterSurface *pSurface = static_cast<VuWaterSurface *>(pWaveNode->mpExtraData);
		pSurface->mpWaveDbrt->remove(pWaveNode);
		pWave->mDbrtNodes.remove(pWaveNode, mPointerFreeList);
	}
}

//*****************************************************************************
void VuWater::rebinWave(VuWaterWave *pWave)
{
	unbinWave(pWave);
	binWave(pWave);
}

//*****************************************************************************
void VuWater::maybeAddWaveToSurface(VuWaterWave *pWave, VuWaterSurface *pSurface)
{
	if ( pWave->mFlags & VuWaterWave::POINT_SURFACE_BINNING )
	{
		// check height
		VuVector3 point = pWave->mBoundingAabb.getCenter();
		if ( pSurface->mWorldAabb.mMin.mZ >= point.mZ || pSurface->mWorldAabb.mMax.mZ <= point.mZ )
			return;
	}
	else
	{
		// check bounding disk
		VuVector3 vWavePos(pWave->mBoundingDiskCenter.mX, pWave->mBoundingDiskCenter.mY, 0);
		float distSquared = pSurface->calcDistance2dSquared(vWavePos);
		if ( distSquared >= pWave->mBoundingDiskRadius*pWave->mBoundingDiskRadius )
			return;

		// check height
		if ( pSurface->mWorldAabb.mMin.mZ >= pWave->mBoundingAabb.mMax.mZ ||
			 pSurface->mWorldAabb.mMax.mZ <= pWave->mBoundingAabb.mMin.mZ )
			return;
	}

	VuDbrtBounds waveBounds;
	waveBounds.mMin = VuVector2(pWave->mBoundingAabb.mMin.mX, pWave->mBoundingAabb.mMin.mY);
	waveBounds.mMax = VuVector2(pWave->mBoundingAabb.mMax.mX, pWave->mBoundingAabb.mMax.mY);

	VuDbrtNode *pNode = pSurface->mpWaveDbrt->insert(pWave, waveBounds);
	pNode->mpExtraData = pSurface;
	pWave->mDbrtNodes.add(pNode, mPointerFreeList);
}

//*****************************************************************************
void VuWater::tickWater(float fdt)
{
	// advance & removed expired waves
	Waves::Node *pNode = mWaves.mpHead;
	while ( pNode )
	{
		Waves::Node *pNext = pNode->mpNext;
		VuWaterWave *pWave = pNode->mpValue;
		if ( !pWave->tick(pWave->mTimeFactor*fdt) )
			removeWave(pWave);
		pNode = pNext;
	}

	// kick off water renderer
	mpRenderer->kick();
}

//*****************************************************************************
void VuWater::initializeOutput(VuWaterSurfaceDataParams &params, float fHeight)
{
	if ( params.mVertexType == VuWaterSurfaceDataParams::VT_PHYSICS )
	{
		VuWaterPhysicsVertex *pVert = params.mpPhysicsVertex;
		for ( int iVert = 0; iVert < params.mVertCount; iVert++ )
		{
			pVert->mDxyzDt = VuVector3(0,0,0);
			pVert->mHeight = fHeight;
			pVert = (VuWaterPhysicsVertex *)((VUBYTE *)pVert + params.mStride);
		}
	}
	else
	{
		VuWaterRenderVertex *pVert = params.mpRenderVertex;
		for ( int iVert = 0; iVert < params.mVertCount; iVert++ )
		{
			pVert->mPosition.mZ = fHeight;
			pVert->mDzDxy = VuPackedVector2(0,0);
			pVert->mFoam = 0.0f;

			pVert = (VuWaterRenderVertex *)((VUBYTE *)pVert + params.mStride);
		}
	}

}

//*****************************************************************************
void VuWater::initializeClipOutput(VuWaterSurfaceDataParams &params, const VuWaterSurface **ppSurfaces, int surfaceCount)
{
	params.mClipType = VuWaterSurfaceDataParams::CT_CLIP;

	// using stack, determine water surface for each vertex
	const VuWaterSurface *stack[MAX_WATER_VERTS_PER_GET];
	params.mppWaterSurface = stack;

	if ( params.mVertexType == VuWaterSurfaceDataParams::VT_PHYSICS )
	{
		VuWaterPhysicsVertex *pVert = params.mpPhysicsVertex;
		for ( int iVert = 0; iVert < params.mVertCount; iVert++ )
		{
			// initialize output
			pVert->mDxyzDt = VuVector3(0,0,0);
			pVert->mHeight = MIN_WORLD_WATER_HEIGHT;

			// determine surface (if any)
			params.mppWaterSurface[iVert] = VUNULL;
			for ( int iSurf = 0; iSurf < surfaceCount; iSurf++ )
			{
				const VuWaterSurface *pSurface = ppSurfaces[iSurf];
				if ( SurfaceIntersects(pSurface, pVert->mPosition) )
				{
					params.mppWaterSurface[iVert] = pSurface;
					pVert->mHeight = pSurface->mDesc.mPos.mZ;
					break;
				}
			}

			// next vert
			pVert = (VuWaterPhysicsVertex *)((VUBYTE *)pVert + params.mStride);
		}
	}
	else
	{
		VuWaterRenderVertex *pVert = params.mpRenderVertex;
		for ( int iVert = 0; iVert < params.mVertCount; iVert++ )
		{
			VuVector3 vPos(pVert->mPosition.mX, pVert->mPosition.mY, pVert->mPosition.mZ);

			// initialize output
			pVert->mPosition.mZ = MIN_WORLD_WATER_HEIGHT;
			pVert->mDzDxy = VuPackedVector2(0,0);
			pVert->mFoam = 0.0f;

			// determine surface (if any)
			params.mppWaterSurface[iVert] = VUNULL;
			for ( int iSurf = 0; iSurf < surfaceCount; iSurf++ )
			{
				const VuWaterSurface *pSurface = ppSurfaces[iSurf];
				if ( SurfaceIntersects(pSurface, vPos) )
				{
					params.mppWaterSurface[iVert] = pSurface;
					pVert->mPosition.mZ = pSurface->mDesc.mPos.mZ;
					break;
				}
			}

			// next vert
			pVert = (VuWaterRenderVertex *)((VUBYTE *)pVert + params.mStride);
		}
	}
}

//*****************************************************************************
void VuWater::updateDevStats(float fdt)
{
	// dev stats
	if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
	{
		if ( pPage->getName() == "Water" )
		{
			// count number of waves types
			typedef std::map<const char *, int> WaveTypes;
			WaveTypes waveTypes;
			for ( Waves::Node *pNode = mWaves.mpHead; pNode; pNode = pNode->mpNext )
				waveTypes[pNode->mpValue->getType()]++;

			pPage->clear();
			int usedCount = mPointerFreeList.getUsedElementCount();
			int freeCount = mPointerFreeList.getFreeElementCount();
			pPage->printf("Pointer Free List: count %d, free %d (%.2f%%)\n", usedCount, freeCount, 100.0f*usedCount/(usedCount + freeCount));
			pPage->printf("Surface Count: %d\n", mSurfaces.size());
			pPage->printf("Shader Count: %d\n", mShaders.size());
			pPage->printf("Total Wave Count: %d\n", mWaves.size());
			for ( WaveTypes::iterator iter = waveTypes.begin(); iter != waveTypes.end(); iter++ )
				pPage->printf("    %s Count: %d\n", iter->first, iter->second);
			pPage->printf("Surface Tree Node Count: %d\n", mpSurfaceDbrt->getNodeCount());
			pPage->printf("Surface Tree Free Count: %d\n", mpSurfaceDbrt->getFreeCount());
			//pPage->printf("Surface Wave Trees:\n");
			//for ( Surfaces::iterator iter = mSurfaces.begin(); iter != mSurfaces.end(); iter++ )
			//	pPage->printf("    Node Count = %3d  Free Count = %3d\n", (*iter)->mpWaveDbrt->getNodeCount(), (*iter)->mpWaveDbrt->getFreeCount());
		}
	}
}

//*****************************************************************************
void VuGetWaveDataPolicy::process(const VuDbrtNode *pNode)
{
	VuWaterWave *pWave = static_cast<VuWaterWave *>(pNode->mpData);

	// ignore?
	for ( int i = 0; i < mParams.mIgnoreWaveCount; i++ )
		if ( pWave == mParams.mapIgnoreWaves[i] )
			return;

	// check sphere
	VuVector2 delta;
	delta.mX = pWave->mBoundingDiskCenter.mX - mParams.mBoundingCenter.mX;
	delta.mY = pWave->mBoundingDiskCenter.mY - mParams.mBoundingCenter.mY;
	float fDistSquared = delta.magSquared();
	float fTotalRadius = pWave->mBoundingDiskRadius + mParams.mBoundingRadius;
	if ( fDistSquared < fTotalRadius*fTotalRadius )
	{
		// apply wave
		mParams.mpWaterClipSurface = static_cast<VuWaterSurface *>(pNode->mpExtraData);
		pWave->getSurfaceData(mParams);
	}
}

//*****************************************************************************
void VuEnumWakeWavesPolicy::process(const VuDbrtNode *pNode)
{
	VuWaterWave *pWave = static_cast<VuWaterWave *>(pNode->mpData);
	if ( pWave->isDerivedFrom(VuWaterWakeWave::msRTTI) )
	{
		// check sphere
		VuVector2 delta;
		delta.mX = pWave->mBoundingDiskCenter.mX - mPos.mX;
		delta.mY = pWave->mBoundingDiskCenter.mY - mPos.mY;
		float fDistSquared = delta.magSquared();
		if ( fDistSquared < pWave->mBoundingDiskRadius*pWave->mBoundingDiskRadius )
		{
			mCallback.enumWakeCB(static_cast<VuWaterWakeWave *>(pWave));
		}
	}
	else if ( pWave->isDerivedFrom(VuWaterFlatWakeWave::msRTTI) )
	{
		// check sphere
		VuVector2 delta;
		delta.mX = pWave->mBoundingDiskCenter.mX - mPos.mX;
		delta.mY = pWave->mBoundingDiskCenter.mY - mPos.mY;
		float fDistSquared = delta.magSquared();
		if ( fDistSquared < pWave->mBoundingDiskRadius*pWave->mBoundingDiskRadius )
		{
			mCallback.enumFlatWakeCB(static_cast<VuWaterFlatWakeWave *>(pWave));
		}
	}
}
