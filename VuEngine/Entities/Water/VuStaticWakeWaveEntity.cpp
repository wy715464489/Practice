//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Static Wake entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Properties/VuPercentageProperty.h"
#include "VuEngine/Water/VuWater.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"


class VuStaticWakeWaveEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuStaticWakeWaveEntity();

	virtual void		onPostLoad() { modified(); }
	virtual void		onGameInitialize();
	virtual void		onGameRelease();

private:
	void				modified();
	void				createWaveParams(VuWaterWakeWaveParams &params0, VuWaterWakeWaveParams &params1);
	void				createWave();
	void				destroyWave();
	void				drawLayout(const Vu3dLayoutDrawParams &params);

	// components
	Vu3dLayoutComponent		*mp3dLayoutComponent;

	// properties
	VuWaterWakeWaveDesc	mWakeWaveDesc;
	float				mFalloffTime;
	float				mDecayTime;
	float				mMagnitude;
	float				mSpeed;
	float				mFrequency;

	VuWaterWakeWave		*mpWave;
};


IMPLEMENT_RTTI(VuStaticWakeWaveEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuStaticWakeWaveEntity);


//*****************************************************************************
VuStaticWakeWaveEntity::VuStaticWakeWaveEntity():
	mFalloffTime(2.0f),
	mDecayTime(1.0f),
	mMagnitude(1.0f),
	mSpeed(10.0f),
	mFrequency(8.0f),
	mpWave(VUNULL)
{
	// properties
	addProperty(new VuPercentageProperty("Range Start Ratio %", mWakeWaveDesc.mRangeStartRatio))	->	setWatcher(this, &VuStaticWakeWaveEntity::modified);
	addProperty(new VuPercentageProperty("Range End Ratio %", mWakeWaveDesc.mRangeDecayRatio))		->	setWatcher(this, &VuStaticWakeWaveEntity::modified);
	addProperty(new VuFloatProperty("Falloff Time", mFalloffTime))	->	setWatcher(this, &VuStaticWakeWaveEntity::modified);
	addProperty(new VuFloatProperty("Decay Time", mDecayTime))		->	setWatcher(this, &VuStaticWakeWaveEntity::modified);
	addProperty(new VuFloatProperty("Magnitude", mMagnitude))		->	setWatcher(this, &VuStaticWakeWaveEntity::modified);
	addProperty(new VuFloatProperty("Speed", mSpeed))				->	setWatcher(this, &VuStaticWakeWaveEntity::modified);
	addProperty(new VuFloatProperty("Frequency", mFrequency))		->	setWatcher(this, &VuStaticWakeWaveEntity::modified);

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	mp3dLayoutComponent->setDrawMethod(this, &VuStaticWakeWaveEntity::drawLayout);

	// want to know when transform is changed
	mpTransformComponent->setWatcher(&VuStaticWakeWaveEntity::modified);

	// limit manipulation to translation, rotation about Z, and scale in x-y
	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT_Z|VuTransformComponent::SCALE_X|VuTransformComponent::SCALE_Y);

	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-1.0f, 0, 0), VuVector3(1.0f, 1.0f, 0)));
}

//*****************************************************************************
void VuStaticWakeWaveEntity::onGameInitialize()
{
	createWave();
}

//*****************************************************************************
void VuStaticWakeWaveEntity::onGameRelease()
{
	destroyWave();
}

//*****************************************************************************
void VuStaticWakeWaveEntity::modified()
{
	if ( isGameInitialized() )
	{
		destroyWave();
		createWave();
	}
}

//*****************************************************************************
void VuStaticWakeWaveEntity::createWaveParams(VuWaterWakeWaveParams &params0, VuWaterWakeWaveParams &params1)
{
	const VuMatrix &transform = mpTransformComponent->getWorldTransform();
	const VuVector3 &scale = mpTransformComponent->getWorldScale();
	const VuVector3 &pos = transform.getTrans();
	const VuVector3 &dir = transform.getAxisY();

	// set node 0 params
	params0.mPosition = pos;
	params0.mDirection = VuVector2(dir.mX, dir.mY);
	params0.mFalloffTime = mFalloffTime;
	params0.mDecayTime = mDecayTime;
	params0.mMagnitude = mMagnitude;
	params0.mRange = scale.mX;
	params0.mSpeed = mSpeed;
	params0.mFrequency = mFrequency;
	params0.mAge = 0;

	// set node 1 params
	params1.mPosition = pos + dir*scale.mY;
	params1.mDirection = VuVector2(dir.mX, dir.mY);
	params1.mFalloffTime = mFalloffTime;
	params1.mDecayTime = mDecayTime;
	params1.mMagnitude = mMagnitude;
	params1.mRange = scale.mX;
	params1.mSpeed = mSpeed;
	params1.mFrequency = mFrequency;
	params1.mAge = mFalloffTime;
}

//*****************************************************************************
void VuStaticWakeWaveEntity::createWave()
{
	VuWaterWakeWaveParams params0, params1;

	createWaveParams(params0, params1);

	mpWave = VuWater::IF()->createWakeWave(mWakeWaveDesc, params0);
	mpWave->update(params1);
	mpWave->setTimeFactor(0);
}

//*****************************************************************************
void VuStaticWakeWaveEntity::destroyWave()
{
	mpWave->removeRef();

	// this is not standard... use with caution
	VUASSERT(mpWave->refCount() == 1, "VuStaticWakeWaveEntity::destroyWave() invalid ref count");
	VuWater::IF()->removeWave(mpWave);

	mpWave = VUNULL;
}

//*****************************************************************************
void VuStaticWakeWaveEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

		VuWaterWakeWaveDesc desc;
		VuWaterWakeWaveParams params0, params1;
		createWaveParams(params0, params1);

		VuColor color = VuColor(255,255,0);

		// draw segment
		pGfxUtil->drawLine3d(color, params0.mPosition, params1.mPosition, params.mCamera.getViewProjMatrix());

		// calculate extents
		VuVector3 vLeft0, vRight0, vLeft1, vRight1;
		VuWaterWakeWave::calculateExtents(desc, params0, params1, vLeft0, vRight0, vLeft1, vRight1);

		// draw outline
		pGfxUtil->drawLine3d(color, vLeft1, vRight1, params.mCamera.getViewProjMatrix());
		pGfxUtil->drawLine3d(color, vLeft0, vLeft1, params.mCamera.getViewProjMatrix());
		pGfxUtil->drawLine3d(color, vRight0, vRight1, params.mCamera.getViewProjMatrix());
	}
}
