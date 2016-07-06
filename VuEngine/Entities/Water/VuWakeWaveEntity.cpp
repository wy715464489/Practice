//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Wake Emitter entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Properties/VuPercentageProperty.h"
#include "VuEngine/Water/VuWater.h"


class VuWakeWaveEntity : public VuEntity, public VuMotionComponentIF
{
	DECLARE_RTTI

public:
	VuWakeWaveEntity();

	virtual void		onGameRelease();

private:
	// VuMotionComponentIF interface
	virtual void		onMotionUpdate();
	virtual void		onMotionDeactivate();

	// components
	VuScriptComponent	*mpScriptComponent;
	VuMotionComponent	*mpMotionComponent;

	// properties
	float				mEmissionRate;
	float				mMinEmissionSpeed;
	float				mMaxEmissionSpeed;
	float				mMagnitude;
	float				mFalloffTime;
	float				mDecayTime;
	float				mRange;
	float				mSpeed;
	float				mFrequency;
	VuWaterWakeWaveDesc	mWakeWaveDesc;

	VuWaterWakeWave		*mpCurWakeWave;
};

IMPLEMENT_RTTI(VuWakeWaveEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuWakeWaveEntity);


//*****************************************************************************
VuWakeWaveEntity::VuWakeWaveEntity():
	mEmissionRate(1.0f),
	mMinEmissionSpeed(0.0f),
	mMaxEmissionSpeed(1.0f),
	mMagnitude(0.25f),
	mFalloffTime(2.0f),
	mDecayTime(1.0f),
	mRange(10.0f),
	mSpeed(10.0f),
	mFrequency(8.0f),
	mpCurWakeWave(VUNULL)
{
	// properties
	addProperty(new VuFloatProperty("Emission Rate", mEmissionRate));
	addProperty(new VuFloatProperty("Min Emission Speed", mMinEmissionSpeed));
	addProperty(new VuFloatProperty("Max Emission Speed", mMaxEmissionSpeed));
	addProperty(new VuPercentageProperty("Range Start Ratio %", mWakeWaveDesc.mRangeStartRatio));
	addProperty(new VuPercentageProperty("Range End Ratio %", mWakeWaveDesc.mRangeDecayRatio));
	addProperty(new VuFloatProperty("Magnitude", mMagnitude));
	addProperty(new VuFloatProperty("Falloff Time", mFalloffTime));
	addProperty(new VuFloatProperty("DecayTime", mDecayTime));
	addProperty(new VuFloatProperty("Range", mRange));
	addProperty(new VuFloatProperty("Speed", mSpeed));
	addProperty(new VuFloatProperty("Frequency", mFrequency));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this));
	addComponent(mpMotionComponent = new VuMotionComponent(this, this));
}

//*****************************************************************************
void VuWakeWaveEntity::onGameRelease()
{
	VUASSERT(mpCurWakeWave == VUNULL, "VuWakeWaveEntity::onGameRelease() dangling wake");
}

//*****************************************************************************
void VuWakeWaveEntity::onMotionUpdate()
{
	mpTransformComponent->setWorldTransform(mpMotionComponent->getWorldTransform());

	// calculate wake params
	VuWaterWakeWaveParams params;
	{
		VuVector3 vWakePos = mpMotionComponent->getWorldTransform().getTrans();
		VuVector3 vWakeVel = mpMotionComponent->getWorldLinearVelocity();
		vWakeVel.mZ = 0;
		float fVel = vWakeVel.mag();
		if ( fVel > 0 )
			vWakeVel /= fVel;

		float fRatio = (fVel - 1.0f)/(mMaxEmissionSpeed - mMinEmissionSpeed);
		fRatio = VuClamp(fRatio, 0.0f, 1.0f);

		params.mPosition = vWakePos;
		params.mDirection = VuVector2(vWakeVel.mX, vWakeVel.mY);
		params.mMagnitude = fRatio*mMagnitude;
		params.mFalloffTime = fRatio*mFalloffTime;
		params.mDecayTime = fRatio*mDecayTime;
		params.mRange = fRatio*mRange;
		params.mSpeed = VuMax(fRatio*mSpeed, FLT_EPSILON);
		params.mFrequency = mFrequency;
	}

	if ( !mpCurWakeWave )
	{
		mpCurWakeWave = VuWater::IF()->createWakeWave(mWakeWaveDesc, params);
		return;
	}

	// update current wake wave
	mpCurWakeWave->update(params);

	// check if we should emit a new wave
	if ( mpCurWakeWave->age() < 1.0f/mEmissionRate )
		return;

	// create new wake wave
	mpCurWakeWave->removeRef();
	mpCurWakeWave = VuWater::IF()->createWakeWave(mWakeWaveDesc, params);
}

//*****************************************************************************
void VuWakeWaveEntity::onMotionDeactivate()
{
	if ( mpCurWakeWave )
	{
		mpCurWakeWave->removeRef();
		mpCurWakeWave = VUNULL;
	}
}
