//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water directional wave class
// 
//*****************************************************************************

#pragma once

#include "VuWaterWave.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuMatrix.h"


//*****************************************************************************
// Description used for wave creation
//*****************************************************************************
class VuWaterDirectionalWaveDesc
{
public:
	VuWaterDirectionalWaveDesc() { memset(this, 0, sizeof(*this)); }

	VuVector3	mPos;
	float		mRotZ;
	float		mSizeX;
	float		mSizeY;
	float		mMaxHeight;
	float		mSpeed;
	float		mFrequency;
	float		mLongitudinalDecayRatio;
	float		mLateralDecayRatio;
};

//*****************************************************************************
// Directional Wave
//*****************************************************************************
class VuWaterDirectionalWave : public VuWaterWave
{
	DECLARE_RTTI

public:
	VuWaterDirectionalWave(const VuWaterDirectionalWaveDesc &desc);

	void						modify(const VuWaterDirectionalWaveDesc &desc);

	// VuWaterWave interface
	virtual	bool				tick(float fdt);
	virtual void				getSurfaceData(VuWaterSurfaceDataParams &params);

	virtual void				debugDraw2d();

private:
	void						setDesc(const VuWaterDirectionalWaveDesc &desc);
	void						updateBounds();

	template<int VERTEX_TYPE, int CLIP_TYPE>
	void						getSurfaceData(VuWaterSurfaceDataParams &params);

	VuWaterDirectionalWaveDesc	mDesc;
	float						mAge;
	VuMatrix					mLocalToWorldMatrix;
	VuMatrix					mWorldToLocalMatrix;

protected:
	~VuWaterDirectionalWave() {}
};
