//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water Bump wave class
// 
//*****************************************************************************

#pragma once

#include "VuWaterWave.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuMatrix.h"


//*****************************************************************************
// Description used for wave creation
//*****************************************************************************
class VuWaterBumpWaveDesc
{
public:
	VuWaterBumpWaveDesc() { memset(this, 0, sizeof(*this)); }

	VuVector3	mPos;
	float		mRotZ;
	float		mSizeX;
	float		mSizeY;
	float		mMaxHeight;
	float		mLateralDecayRatio;
};

//*****************************************************************************
// Bump Wave
//*****************************************************************************
class VuWaterBumpWave : public VuWaterWave
{
	DECLARE_RTTI

public:
	VuWaterBumpWave(const VuWaterBumpWaveDesc &desc);

	void						modify(const VuWaterBumpWaveDesc &desc);

	// VuWaterWave interface
	virtual	bool				tick(float fdt);
	virtual void				getSurfaceData(VuWaterSurfaceDataParams &params);

	virtual void				debugDraw2d();

private:
	void						setDesc(const VuWaterBumpWaveDesc &desc);
	void						updateBounds();

	template<int VERTEX_TYPE, int CLIP_TYPE>
	void						getSurfaceData(VuWaterSurfaceDataParams &params);

	VuWaterBumpWaveDesc	mDesc;
	VuMatrix					mLocalToWorldMatrix;
	VuMatrix					mWorldToLocalMatrix;

protected:
	~VuWaterBumpWave() {}
};
