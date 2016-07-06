//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water whirlpool wave class
// 
//*****************************************************************************

#pragma once

#include "VuWaterWave.h"
#include "VuEngine/Math/VuVector3.h"


//*****************************************************************************
// Description used for wave creation
//*****************************************************************************
class VuWaterWhirlpoolWaveDesc
{
public:
	VuWaterWhirlpoolWaveDesc() { memset(this, 0, sizeof(*this)); }

	VuVector3	mPos;
	float		mOuterRadius;
	float		mInnerRadius;
	float		mDepth;
	float		mAngularSpeed;
	float		mLinearSpeed;
	float		mFoaminess;
};

//*****************************************************************************
// Whirlpool wave
//*****************************************************************************
class VuWaterWhirlpoolWave : public VuWaterWave
{
	DECLARE_RTTI

public:
	VuWaterWhirlpoolWave(const VuWaterWhirlpoolWaveDesc &desc);

	void						modify(const VuWaterWhirlpoolWaveDesc &desc);

	// VuWaterWaveImpl interface
	virtual	bool				tick(float fdt);
	virtual void				getSurfaceData(VuWaterSurfaceDataParams &params);

	virtual void				debugDraw3d(const VuCamera &camera);
	virtual void				debugDraw2d();

private:
	void						setDesc(const VuWaterWhirlpoolWaveDesc &desc);
	void						updateBounds();

	template<int VERTEX_TYPE, int CLIP_TYPE>
	void						getSurfaceData(VuWaterSurfaceDataParams &params);

	VuWaterWhirlpoolWaveDesc	mDesc;
	float						mAge;

protected:
	~VuWaterWhirlpoolWave() {}
};
