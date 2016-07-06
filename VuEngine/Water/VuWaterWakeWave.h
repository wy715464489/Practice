//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water wake wave implementation
// 
//*****************************************************************************

#pragma once

#include "VuWaterWave.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuVector2.h"


//*****************************************************************************
// Params used to update wake waves
//*****************************************************************************
class VuWaterWakeWaveParams
{
public:
	VuWaterWakeWaveParams() { memset(this, 0, sizeof(*this)); }

	// variables
	VuVector3	mPosition;
	VuVector2	mDirection;
	float		mFalloffTime;
	float		mDecayTime;
	float		mMagnitude;
	float		mRange;
	float		mSpeed;
	float		mFrequency;

	float		mAge;
};

//*****************************************************************************
// Description used for wave creation
//*****************************************************************************
class VuWaterWakeWaveDesc
{
public:
	VuWaterWakeWaveDesc() : mRangeStartRatio(0.2f), mRangeDecayRatio(0.8f), mPhysicsScale(1.0f) {}

	float					mRangeStartRatio;
	float					mRangeDecayRatio;
	float					mPhysicsScale;
};

//*****************************************************************************
// Water wake wave interface
//*****************************************************************************
class VuWaterWakeWave : public VuWaterWave
{
	DECLARE_RTTI

public:
	VuWaterWakeWave(const VuWaterWakeWaveDesc &desc, const VuWaterWakeWaveParams &params);

	void				update(const VuWaterWakeWaveParams &params);
	float				age() { return mNode1.mAge; }
	bool				interpolate(const VuVector2 &vPos, VuWaterWakeWaveParams &params) const;

	// VuWaterWave interface
	virtual	bool		tick(float fdt);
	virtual void		getSurfaceData(VuWaterSurfaceDataParams &params);

	virtual void		debugDraw3d(const VuCamera &camera);
	virtual void		debugDraw2d();

	// utility
	static void			calculateExtents(const VuWaterWakeWaveDesc &desc, const VuWaterWakeWaveParams &node0, const VuWaterWakeWaveParams &node1,
							VuVector3 &vLeft0, VuVector3 &vRight0, VuVector3 &vLeft1, VuVector3 &vRight1);
protected:
	void					updateBounds();

	template<int VERTEX_TYPE, int CLIP_TYPE>
	void					getSurfaceData(VuWaterSurfaceDataParams &node);

	VuWaterWakeWaveDesc		mDesc;
	VuWaterWakeWaveParams	mNode0;
	VuWaterWakeWaveParams	mNode1;

protected:
	~VuWaterWakeWave() {}
};


//*****************************************************************************
// Water flat wake wave interface
//*****************************************************************************
class VuWaterFlatWakeWave : public VuWaterWave
{
	DECLARE_RTTI

public:
	VuWaterFlatWakeWave(const VuWaterWakeWaveDesc &desc, const VuWaterWakeWaveParams &params);

	void				update(const VuWaterWakeWaveParams &params);
	float				age() { return mNode1.mAge; }

	// VuWaterWave interface
	virtual	bool		tick(float fdt);
	virtual void		getSurfaceData(VuWaterSurfaceDataParams &params);

	virtual void		debugDraw3d(const VuCamera &camera);
	virtual void		debugDraw2d();

	// utility
	static void			calculateExtents(const VuWaterWakeWaveDesc &desc, const VuWaterWakeWaveParams &node0, const VuWaterWakeWaveParams &node1,
							VuVector3 &vLeft0, VuVector3 &vRight0, VuVector3 &vLeft1, VuVector3 &vRight1);
protected:
	void					updateBounds();

	template<int VERTEX_TYPE, int CLIP_TYPE>
	void					getSurfaceData(VuWaterSurfaceDataParams &node);

	VuWaterWakeWaveDesc		mDesc;
	VuWaterWakeWaveParams	mNode0;
	VuWaterWakeWaveParams	mNode1;

protected:
	~VuWaterFlatWakeWave() {}
};
