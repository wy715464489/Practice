//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water directional flow wave class
// 
//*****************************************************************************

#pragma once

#include "VuWaterWave.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuMatrix.h"


//*****************************************************************************
// Description used for wave creation
//*****************************************************************************
class VuWaterDirectionalFlowWaveDesc
{
public:
	VuWaterDirectionalFlowWaveDesc() { memset(this, 0, sizeof(*this)); }

	VuVector3	mPos;
	float		mRotZ;
	float		mSizeX;
	float		mSizeY;
	VuVector3	mFlowVelocity;
	float		mLongitudinalDecayRatio;
	float		mLateralDecayRatio;
};

//*****************************************************************************
// Directional flow wave
//*****************************************************************************
class VuWaterDirectionalFlowWave : public VuWaterWave
{
	DECLARE_RTTI

public:
	VuWaterDirectionalFlowWave(const VuWaterDirectionalFlowWaveDesc &desc);

	void							modify(const VuWaterDirectionalFlowWaveDesc &desc);

	// VuWaterWave interface
	virtual bool					tick(float fdt);
	virtual void					getSurfaceData(VuWaterSurfaceDataParams &params);
	virtual void					debugDraw2d();

private:
	void							setDesc(const VuWaterDirectionalFlowWaveDesc &desc);
	void							updateBounds();

	template<int CLIP_TYPE>
	void							getSurfaceData(VuWaterSurfaceDataParams &params);

	VuWaterDirectionalFlowWaveDesc	mDesc;
	VuMatrix						mLocalToWorldMatrix;
	VuMatrix						mWorldToLocalMatrix;

protected:
	~VuWaterDirectionalFlowWave() {}
};
