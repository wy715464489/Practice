//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Water banked turn wave class
// 
//*****************************************************************************

#pragma once

#include "VuWaterWave.h"
#include "VuEngine/Math/VuVector3.h"


//*****************************************************************************
// Description used for wave creation
//*****************************************************************************
class VuWaterBankedTurnWaveDesc
{
public:
	VuWaterBankedTurnWaveDesc() { memset(this, 0, sizeof(*this)); }

	VuVector3	mPos;
	float		mRotZ;
	float		mOuterRadius;
	float		mInnerRadius;
	float		mHeight;
	float		mAngularSize;
	float		mAngularDecayRatio;
};

//*****************************************************************************
// BankedTurn wave
//*****************************************************************************
class VuWaterBankedTurnWave : public VuWaterWave
{
	DECLARE_RTTI

public:
	VuWaterBankedTurnWave(const VuWaterBankedTurnWaveDesc &desc);

	void						modify(const VuWaterBankedTurnWaveDesc &desc);

	// VuWaterWaveImpl interface
	virtual	bool				tick(float fdt);
	virtual void				getSurfaceData(VuWaterSurfaceDataParams &params);

	virtual void				debugDraw3d(const VuCamera &camera);
	virtual void				debugDraw2d();

private:
	void						setDesc(const VuWaterBankedTurnWaveDesc &desc);
	void						updateBounds();

	template<int VERTEX_TYPE, int CLIP_TYPE>
	void						getSurfaceData(VuWaterSurfaceDataParams &params);

	VuWaterBankedTurnWaveDesc	mDesc;
	VuVector2					mFwd;
	float						mHalfAngularSize;

protected:
	~VuWaterBankedTurnWave() {}
};
