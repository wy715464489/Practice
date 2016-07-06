//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Whirlpool entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"

class VuWaterWhirlpoolWaveDesc;
class Vu3dLayoutDrawParams;
class Vu3dLayoutComponent;
class VuWaterWhirlpoolWave;


class VuWhirlpoolWaveEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuWhirlpoolWaveEntity();

	virtual void			onPostLoad() { modified(); }
	virtual void			onGameInitialize();
	virtual void			onGameRelease();

protected:
	virtual void			modified();
	void					createWaveDesc(VuWaterWhirlpoolWaveDesc &desc);
	void					drawLayout(const Vu3dLayoutDrawParams &params);

	Vu3dLayoutComponent		*mp3dLayoutComponent;

	float					mOuterRadius;
	float					mInnerRadius;
	float					mDepth;
	float					mAngularSpeed;
	float					mLinearSpeed;
	float					mFoaminess;

	VuWaterWhirlpoolWave	*mpWave;
};

