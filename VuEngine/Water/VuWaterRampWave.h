//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water ramp wave class
// 
//*****************************************************************************

#pragma once

#include "VuWaterWave.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuMatrix.h"


//*****************************************************************************
// Description used for wave creation
//*****************************************************************************
class VuWaterRampWaveDesc
{
public:
	VuWaterRampWaveDesc() { memset(this, 0, sizeof(*this)); }

	VuVector3	mPos;
	VuVector3	mSize;
	float		mRotZ;
	float		mTransitionRatio;
	float		mFlowSpeed;
};

//*****************************************************************************
// Ramp wave
//*****************************************************************************
class VuWaterRampWave : public VuWaterWave
{
	DECLARE_RTTI

public:
	VuWaterRampWave(const VuWaterRampWaveDesc &desc);

	void				modify(const VuWaterRampWaveDesc &desc);

	// VuWaterWave interface
	virtual	bool		tick(float fdt);
	virtual void		getSurfaceData(VuWaterSurfaceDataParams &params);
	virtual void		debugDraw2d();

private:
	void				setDesc(const VuWaterRampWaveDesc &desc);
	void				updateBounds();

	template<int VERTEX_TYPE, int CLIP_TYPE>
	void				getSurfaceData(VuWaterSurfaceDataParams &params);

	VuWaterRampWaveDesc	mDesc;
	VuMatrix			mLocalToWorldMatrix;
	VuMatrix			mWorldToLocalMatrix;
	float				mFactor;
	float				mSlope;
	VuVector2			mNormal;
	VuVector3			mAxisX;
	VuVector3			mSlopedFlow;
	VuVector3			mStraightFlow;

protected:
	~VuWaterRampWave() {}
};
