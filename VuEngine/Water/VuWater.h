//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water interface
// 
//*****************************************************************************

#pragma once

#include "VuWaterSurface.h"
#include "VuWaterShader.h"
#include "VuWaterRenderer.h"
#include "VuWaterWakeWave.h"
#include "VuWaterDirectionalWave.h"
#include "VuWaterDirectionalFlowWave.h"
#include "VuWaterOceanWave.h"
#include "VuWaterWhirlpoolWave.h"
#include "VuWaterRampWave.h"
#include "VuWaterPointWave.h"
#include "VuWaterBumpWave.h"
#include "VuWaterBankedTurnWave.h"
#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Containers/VuPointerList.h"
#include "VuEngine/Math/VuPackedVector.h"
#include "VuEngine/Math/VuAabb.h"

class VuWaterSurfaceDataParams;
class VuWaterPhysicsVertex;
class VuWaterRenderVertex;
class VuWaterSurfaceCallback;
class VuWaterDebugDrawer;
class VuDbrt;


#define MIN_WORLD_WATER_HEIGHT	(-0.5f*FLT_MAX)


//*****************************************************************************
// Water
//*****************************************************************************
class VuWater : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuWater)

protected:
	// created/destroyed by action game mode
	friend class VuEngine;
	virtual bool init(bool bAsynchronous);
	virtual void release();

public:
	VuWater();

	//*************************************
	// public interface
	//*************************************

	// water renderer
	VuWaterRenderer				*renderer();

	// water surfaces
	VuWaterSurface				*createSurface(const VuWaterSurfaceDesc &desc, VuEntity *pOwnerEntity);

	// water shaders
	VuWaterShader				*createShader(const VuWaterShaderDesc &desc);

	// waves
	VuWaterWakeWave				*createWakeWave(const VuWaterWakeWaveDesc &desc, const VuWaterWakeWaveParams &params);
	VuWaterFlatWakeWave			*createFlatWakeWave(const VuWaterWakeWaveDesc &desc, const VuWaterWakeWaveParams &params);
	VuWaterDirectionalWave		*createDirectionalWave(const VuWaterDirectionalWaveDesc &desc);
	VuWaterDirectionalFlowWave	*createDirectionalFlowWave(const VuWaterDirectionalFlowWaveDesc &desc);
	VuWaterInfiniteOceanWave	*createInfiniteOceanWave(const VuWaterInfiniteOceanWaveDesc &desc);
	VuWaterRectangularOceanWave	*createRectangularOceanWave(const VuWaterRectangularOceanWaveDesc &desc);
	VuWaterCircularOceanWave	*createCircularOceanWave(const VuWaterCircularOceanWaveDesc &desc);
	VuWaterWhirlpoolWave		*createWhirlpoolWave(const VuWaterWhirlpoolWaveDesc &desc);
	VuWaterRampWave				*createRampWave(const VuWaterRampWaveDesc &desc);
	VuWaterPointWave			*createPointWave(const VuWaterPointWaveDesc &desc);
	VuWaterBumpWave				*createBumpWave(const VuWaterBumpWaveDesc &desc);
	VuWaterBankedTurnWave		*createBankedTurnWave(const VuWaterBankedTurnWaveDesc &desc);

	// custom waves
	void						addCustomWave(VuWaterWave *pWave);

	// obtaining water surface information
	void						getSurfaceData(VuWaterSurfaceDataParams &params);
	const VuWaterSurface		*getSurface(const VuVector3 &vPos, bool bIgnoreHeight = false);

	// convenience
	VuWaterPhysicsVertex		getPhysicsVertex(const VuVector3 &vPosition);
	VuWaterRenderVertex			getRenderVertex(const VuVector3 &vPosition);

	// get water surface z coordinate
	float						getWaterSurfaceZ(const VuVector3 &vPosition, const VuAabb &aabb);
	void						getWaterSurfaceReflectionZ(const VuVector3 &vPosition, const VuAabb &aabb, float &z, float &dist);
	bool						getWaterSurfaceMinZ(float &minZ);

	// enumeration
	class VuWakeWaveEnumCB
	{
	public:
		virtual void enumWakeCB(const VuWaterWakeWave *pWakeWave) {};
		virtual void enumFlatWakeCB(const VuWaterFlatWakeWave *pFlatWakeWave) {};
	};
	void						enumWakeWaves(const VuVector3 &vPosition, VuWakeWaveEnumCB &cb);

	// callbacks
	void						registerSurfaceCallback(VuWaterSurfaceCallback *pCallback)		{ mSurfaceCallbacks.push_back(pCallback); }
	void						unregisterSurfaceCallback(VuWaterSurfaceCallback *pCallback)	{ mSurfaceCallbacks.remove(pCallback); }

	// configuration
	float						getDetail()                                   { return mDetail; }
	bool						getProdecuralReflectionsEnabled()             { return mProceduralReflectionsEnabled; }
	bool						getWakesEnabled()                             { return mWakesEnabled; }
	bool						getNormalMapEnabled()                         { return mNormalMapEnabled; }


	//*************************************
	// internal interface
	//*************************************
	typedef std::list<VuWaterSurface *> Surfaces;
	typedef std::list<VuWaterShader *> Shaders;
	typedef VuPointerList<VuWaterWave> Waves;

	void					removeSurface(VuWaterSurface *pSurface);
	void					removeShader(VuWaterShader *pShader);
	void					removeWave(VuWaterWave *pWave);

	void					binSurface(VuWaterSurface *pSurface);
	void					unbinSurface(VuWaterSurface *pSurface);
	void					rebinSurface(VuWaterSurface *pSurface);

	void					binWave(VuWaterWave *pWave);
	void					unbinWave(VuWaterWave *pWave);
	void					rebinWave(VuWaterWave *pWave);

	void					maybeAddWaveToSurface(VuWaterWave *pWave, VuWaterSurface *pSurface);

	const Surfaces			&getSurfaces()	const	{ return mSurfaces; }
	const Waves				&getWaves()	const		{ return mWaves; }
	const VuDbrt			*getSurfaceDbrt() const	{ return mpSurfaceDbrt; }

private:
	typedef std::list<VuWaterSurfaceCallback *> SurfaceCallbacks;

	void					tickWater(float fdt);

	void					initializeOutput(VuWaterSurfaceDataParams &params, float fHeight);
	void					initializeClipOutput(VuWaterSurfaceDataParams &params, const VuWaterSurface **ppSurfaces, int surfaceCount);
	void					updateDevStats(float fdt);

	void					configDetail(float value) { mDetail = value; }
	void					configReflection(bool value) { mProceduralReflectionsEnabled = value; }
	void					configWakes(bool value) { mWakesEnabled = value; }
	void					configNormalMap(bool value) { mNormalMapEnabled = value; }

	VuPointerFreeList	mPointerFreeList;

	VuWaterDebugDrawer	*mpDebugDrawer;

	bool				mbDisableWaves;

	VuWaterRenderer		*mpRenderer;
	Surfaces			mSurfaces;
	Shaders				mShaders;
	Waves				mWaves;

	SurfaceCallbacks	mSurfaceCallbacks;
	VuDbrt				*mpSurfaceDbrt;

	// configuration
	float				mDetail;
	bool				mProceduralReflectionsEnabled;
	bool				mWakesEnabled;
	bool				mNormalMapEnabled;
};

//*****************************************************************************
// Params used for obtaining water surface data
//*****************************************************************************
class VuWaterSurfaceDataParams
{
public:
	enum eVertexType { VT_PHYSICS, VT_RENDER };
	enum eClipType { CT_NOCLIP, CT_CLIP };
	enum { MAX_IGNORE_WAVE_COUNT = 4 };

	VuWaterSurfaceDataParams(eVertexType vertexType) :
		mIgnoreWaveCount(0), mpWaterSurfaceHint(VUNULL),
		mppWaterSurface(VUNULL), mpWaterClipSurface(VUNULL),
		mVertexType(vertexType), mClipType(CT_NOCLIP)
	{}

	int						mVertCount;

	VuAabb					mBoundingAabb;
	VuVector3				mBoundingCenter;
	float					mBoundingRadius;

	int						mIgnoreWaveCount;
	const VuWaterWave		*mapIgnoreWaves[MAX_IGNORE_WAVE_COUNT];

	const VuWaterSurface	*mpWaterSurfaceHint;

	union
	{
		VuWaterPhysicsVertex	*mpPhysicsVertex;
		VuWaterRenderVertex		*mpRenderVertex;
	};
	int						mStride;


	// internal

	const VuWaterSurface	**mppWaterSurface;
	const VuWaterSurface	*mpWaterClipSurface;

	eVertexType				mVertexType;
	eClipType				mClipType;
};


//*****************************************************************************
// Callback for water surfaces being created/destroyed.
//*****************************************************************************
class VuWaterSurfaceCallback
{
public:
	virtual void	onWaterSurfaceCreated(const VuWaterSurface *pWS)	{}
	virtual void	onWaterSurfaceDestroyed(const VuWaterSurface *pWS)	{}
};

