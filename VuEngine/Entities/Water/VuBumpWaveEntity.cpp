//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Wave generator entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Properties/VuPercentageProperty.h"
#include "VuEngine/Water/VuWater.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Math/VuMatrix.h"


class VuBumpWaveEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuBumpWaveEntity();

	virtual void			onPostLoad() { modified(); }
	virtual void			onGameInitialize();
	virtual void			onGameRelease();

private:
	void					modified();
	void					createWaveDesc(VuWaterBumpWaveDesc &desc);
	void					drawLayout(const Vu3dLayoutDrawParams &params);

	Vu3dLayoutComponent		*mp3dLayoutComponent;

	float					mMaxHeight;
	float					mLateralDecayRatio;

	VuWaterBumpWave			*mpWave;
};


IMPLEMENT_RTTI(VuBumpWaveEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuBumpWaveEntity);


//*****************************************************************************
VuBumpWaveEntity::VuBumpWaveEntity():
	mMaxHeight(1),
	mLateralDecayRatio(0.5),
	mpWave(VUNULL)
{
	// properties
	addProperty(new VuFloatProperty("Max Height", mMaxHeight))										->	setWatcher(this, &VuBumpWaveEntity::modified);
	addProperty(new VuPercentageProperty("Lateral Decay Ratio %", mLateralDecayRatio))				->	setWatcher(this, &VuBumpWaveEntity::modified);

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	mp3dLayoutComponent->setDrawMethod(this, &VuBumpWaveEntity::drawLayout);

	// want to know when transform is changed
	mpTransformComponent->setWatcher(&VuBumpWaveEntity::modified);

	// limit manipulation to translation, rotation about Z, and scale in x-y
	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT_Z|VuTransformComponent::SCALE_X|VuTransformComponent::SCALE_Y);
}

//*****************************************************************************
void VuBumpWaveEntity::onGameInitialize()
{
	// create wave generator
	VuWaterBumpWaveDesc desc;
	createWaveDesc(desc);
	mpWave = VuWater::IF()->createBumpWave(desc);
}

//*****************************************************************************
void VuBumpWaveEntity::onGameRelease()
{
	// destroy wave generator
	mpWave->removeRef();

	// this is not standard... use with caution
	VUASSERT(mpWave->refCount() == 1, "VuBumpWaveEntity::onGameRelease() invalid ref count");
	VuWater::IF()->removeWave(mpWave);

	mpWave = VUNULL;
}

//*****************************************************************************
void VuBumpWaveEntity::modified()
{
	// modify wave
	if ( mpWave )
	{
		VuWaterBumpWaveDesc desc;
		createWaveDesc(desc);
		mpWave->modify(desc);
	}

	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-0.5f, -0.5f, 0.0f), VuVector3(0.5f, 0.5f, mMaxHeight)));
}

//*****************************************************************************
void VuBumpWaveEntity::createWaveDesc(VuWaterBumpWaveDesc &desc)
{
	desc.mPos = mpTransformComponent->getWorldPosition();
	desc.mRotZ = mpTransformComponent->getWorldRotation().mZ;
	desc.mSizeX = mpTransformComponent->getWorldScale().mX;
	desc.mSizeY = mpTransformComponent->getWorldScale().mY;
	desc.mMaxHeight = mMaxHeight;
	desc.mLateralDecayRatio = mLateralDecayRatio;
}

//*****************************************************************************
void VuBumpWaveEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

		VuVector3 vScale = mpTransformComponent->getWorldScale();
		VuMatrix matMVP = mpTransformComponent->getWorldTransform();
		matMVP.scaleLocal(VuVector3(0.5f*vScale.mX, 0.5f*vScale.mY, mMaxHeight));
		matMVP *= params.mCamera.getViewProjMatrix();

		// draw lateral decay ratio
		{
			VuColor col(255,64,64);
			float rx = mLateralDecayRatio;
			pGfxUtil->drawLine3d(col, VuVector3(-rx, -1, 1), VuVector3(-rx,  1, 1), matMVP);
			pGfxUtil->drawLine3d(col, VuVector3( rx, -1, 1), VuVector3( rx,  1, 1), matMVP);
		}
	}
}
