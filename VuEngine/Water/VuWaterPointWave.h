//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water point wave class
// 
//*****************************************************************************

#pragma once

#include "VuWaterWave.h"
#include "VuEngine/Math/VuVector3.h"


//*****************************************************************************
// Description used for wave creation
//*****************************************************************************
class VuWaterPointWaveDesc
{
public:
	VuWaterPointWaveDesc() { memset(this, 0, sizeof(*this)); }

	VuVector3	mPos;
	float		mFalloffTime;
	float		mMagnitude;
	float		mRangeStart;
	float		mRangeEnd;
	float		mSpeed;
	float		mFrequency;
	float		mFoaminess;
};

//*****************************************************************************
class VuPointWaveCallbackIF
{
public:
	virtual void	onPointWaveExpired()	{}
};

//*****************************************************************************
// Point Wave
//*****************************************************************************
class VuWaterPointWave : public VuWaterWave
{
	DECLARE_RTTI

public:
	VuWaterPointWave(const VuWaterPointWaveDesc &desc);

	// VuWaterWave interface
	virtual	bool			tick(float fdt);
	virtual void			getSurfaceData(VuWaterSurfaceDataParams &params);

	virtual void			debugDraw2d();

	void					setCallback(VuPointWaveCallbackIF *pCallbackIF)	{ mpCallbackIF = pCallbackIF; }

private:
	void					setDesc(const VuWaterPointWaveDesc &desc);
	void					updateBounds();

	template<int VERTEX_TYPE, int CLIP_TYPE>
	void					getSurfaceData(VuWaterSurfaceDataParams &params);

	VuWaterPointWaveDesc	mDesc;
	float					mAge;
	float					mInvSpeed;
	float					mInvRangeEndMinusRangeStart;
	float					mInvFalloffTime;
	float					mInvMagnitude;

	VuPointWaveCallbackIF	*mpCallbackIF;
	bool					mbExpired;

protected:
	~VuWaterPointWave() {}
};
