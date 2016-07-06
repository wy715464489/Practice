//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Wave generator entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuPercentageProperty.h"
#include "VuEngine/Water/VuWater.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Math/VuMatrix.h"


class VuDirectionalWaveEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuDirectionalWaveEntity();

	virtual void			onPostLoad() { modified(); }
	virtual void			onGameInitialize();
	virtual void			onGameRelease();

private:
	void					modified();
	void					createWaveDesc(VuWaterDirectionalWaveDesc &desc);
	void					drawLayout(const Vu3dLayoutDrawParams &params);

	Vu3dLayoutComponent		*mp3dLayoutComponent;
	VuScriptComponent		*mpScriptComponent;

	float					mMaxHeight;
	float					mSpeed;
	float					mPeriod;
	float					mLongitudinalDecayRatio;
	float					mLateralDecayRatio;

	VuWaterDirectionalWave	*mpWave;
};


IMPLEMENT_RTTI(VuDirectionalWaveEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuDirectionalWaveEntity);


//*****************************************************************************
VuDirectionalWaveEntity::VuDirectionalWaveEntity():
	mMaxHeight(1),
	mSpeed(1),
	mPeriod(1),
	mLongitudinalDecayRatio(0.5),
	mLateralDecayRatio(0.5),
	mpWave(VUNULL)
{
	// properties
	addProperty(new VuFloatProperty("Max Height", mMaxHeight))										->	setWatcher(this, &VuDirectionalWaveEntity::modified);
	addProperty(new VuFloatProperty("Speed", mSpeed))												->	setWatcher(this, &VuDirectionalWaveEntity::modified);
	addProperty(new VuFloatProperty("Period", mPeriod))												->	setWatcher(this, &VuDirectionalWaveEntity::modified);
	addProperty(new VuPercentageProperty("Longitudinal Decay Ratio %", mLongitudinalDecayRatio))	->	setWatcher(this, &VuDirectionalWaveEntity::modified);
	addProperty(new VuPercentageProperty("Lateral Decay Ratio %", mLateralDecayRatio))				->	setWatcher(this, &VuDirectionalWaveEntity::modified);

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150, false));

	// want to know when transform is changed
	mpTransformComponent->setWatcher(&VuDirectionalWaveEntity::modified);

	// limit manipulation to translation, rotation about Z, and scale in x-y
	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT_Z|VuTransformComponent::SCALE_X|VuTransformComponent::SCALE_Y);

	mp3dLayoutComponent->setDrawMethod(this, &VuDirectionalWaveEntity::drawLayout);
}

//*****************************************************************************
void VuDirectionalWaveEntity::onGameInitialize()
{
	// create wave generator
	VuWaterDirectionalWaveDesc desc;
	createWaveDesc(desc);
	mpWave = VuWater::IF()->createDirectionalWave(desc);
}

//*****************************************************************************
void VuDirectionalWaveEntity::onGameRelease()
{
	// destroy wave generator
	mpWave->removeRef();

	// this is not standard... use with caution
	VUASSERT(mpWave->refCount() == 1, "VuDirectionalWaveEntity::onGameRelease() invalid ref count");
	VuWater::IF()->removeWave(mpWave);

	mpWave = VUNULL;
}

//*****************************************************************************
void VuDirectionalWaveEntity::modified()
{
	// modify wave
	if ( mpWave )
	{
		VuWaterDirectionalWaveDesc desc;
		createWaveDesc(desc);
		mpWave->modify(desc);
	}

	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-0.5f, -0.5f, -mMaxHeight), VuVector3(0.5f, 0.5f, mMaxHeight)));
}

//*****************************************************************************
void VuDirectionalWaveEntity::createWaveDesc(VuWaterDirectionalWaveDesc &desc)
{
	desc.mPos = mpTransformComponent->getWorldPosition();
	desc.mRotZ = mpTransformComponent->getWorldRotation().mZ;
	desc.mSizeX = mpTransformComponent->getWorldScale().mX;
	desc.mSizeY = mpTransformComponent->getWorldScale().mY;
	desc.mMaxHeight = mMaxHeight;
	desc.mSpeed = mSpeed/mPeriod;
	desc.mFrequency = 0.5f*desc.mSizeY/mPeriod;
	desc.mLongitudinalDecayRatio = mLongitudinalDecayRatio;
	desc.mLateralDecayRatio = mLateralDecayRatio;
}

//*****************************************************************************
void VuDirectionalWaveEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

		VuVector3 vScale = mpTransformComponent->getWorldScale();
		VuMatrix matMVP = mpTransformComponent->getWorldTransform();
		matMVP.scaleLocal(VuVector3(0.5f*vScale.mX, 0.5f*vScale.mY, mMaxHeight));
		matMVP *= params.mCamera.getViewProjMatrix();

		// draw waves
		{
			VuColor col(128,128,128);
			float fPeriod = mPeriod/(0.5f*vScale.mY);
			for ( float y = 0; y < 1; y += fPeriod )
			{
				pGfxUtil->drawLine3d(col, VuVector3(-1,  y, 1), VuVector3(1,  y, 1), matMVP);
				pGfxUtil->drawLine3d(col, VuVector3(-1, -y, 1), VuVector3(1, -y, 1), matMVP);
			}
		}

		// draw decay ratios
		{
			VuColor col(255,64,64);
			float rx = mLateralDecayRatio;
			float ry = mLongitudinalDecayRatio;
			pGfxUtil->drawLine3d(col, VuVector3(-rx, -ry, 1), VuVector3( rx, -ry, 1), matMVP);
			pGfxUtil->drawLine3d(col, VuVector3(-rx,  ry, 1), VuVector3( rx,  ry, 1), matMVP);
			pGfxUtil->drawLine3d(col, VuVector3(-rx, -ry, 1), VuVector3(-rx,  ry, 1), matMVP);
			pGfxUtil->drawLine3d(col, VuVector3( rx, -ry, 1), VuVector3( rx,  ry, 1), matMVP);
		}
	}
}
