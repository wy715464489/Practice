//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Whirlpool entity
// 
//*****************************************************************************

#include "VuWhirlpoolWaveEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Properties/VuAngleProperty.h"
#include "VuEngine/Water/VuWater.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Math/VuMatrix.h"


IMPLEMENT_RTTI(VuWhirlpoolWaveEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuWhirlpoolWaveEntity);


//*****************************************************************************
VuWhirlpoolWaveEntity::VuWhirlpoolWaveEntity():
	mOuterRadius(20),
	mInnerRadius(5),
	mDepth(10),
	mAngularSpeed(0),
	mLinearSpeed(0),
	mFoaminess(1),
	mpWave(VUNULL)
{
	// properties
	addProperty(new VuFloatProperty("Inner Radius", mInnerRadius))		->	setWatcher(this, &VuWhirlpoolWaveEntity::modified);
	addProperty(new VuFloatProperty("Outer Radius", mOuterRadius))		->	setWatcher(this, &VuWhirlpoolWaveEntity::modified);
	addProperty(new VuFloatProperty("Depth", mDepth))					->	setWatcher(this, &VuWhirlpoolWaveEntity::modified);
	addProperty(new VuFloatProperty("Angular Speed", mAngularSpeed))	->	setWatcher(this, &VuWhirlpoolWaveEntity::modified);
	addProperty(new VuFloatProperty("Linear Speed", mLinearSpeed))		->	setWatcher(this, &VuWhirlpoolWaveEntity::modified);
	addProperty(new VuFloatProperty("Foaminess", mFoaminess))			->	setWatcher(this, &VuWhirlpoolWaveEntity::modified);

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	mp3dLayoutComponent->setDrawMethod(this, &VuWhirlpoolWaveEntity::drawLayout);

	// want to know when transform is changed
	mpTransformComponent->setWatcher(&VuWhirlpoolWaveEntity::modified);

	// limit manipulation to translation
	mpTransformComponent->setMask(VuTransformComponent::TRANS);

	modified();
}

//*****************************************************************************
void VuWhirlpoolWaveEntity::onGameInitialize()
{
	// create wave generator
	VuWaterWhirlpoolWaveDesc desc;
	createWaveDesc(desc);
	mpWave = VuWater::IF()->createWhirlpoolWave(desc);
}

//*****************************************************************************
void VuWhirlpoolWaveEntity::onGameRelease()
{
	// destroy wave generator
	mpWave->removeRef();

	// this is not standard... use with caution
	VUASSERT(mpWave->refCount() == 1, "VuWhirlpoolWaveEntity::onGameRelease() invalid ref count");
	VuWater::IF()->removeWave(mpWave);

	mpWave = VUNULL;
}

//*****************************************************************************
void VuWhirlpoolWaveEntity::modified()
{
	// modify wave
	if ( mpWave )
	{
		VuWaterWhirlpoolWaveDesc desc;
		createWaveDesc(desc);
		mpWave->modify(desc);
	}

	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-mOuterRadius, -mOuterRadius, -mDepth), VuVector3(mOuterRadius, mOuterRadius, 0)));
}

//*****************************************************************************
void VuWhirlpoolWaveEntity::createWaveDesc(VuWaterWhirlpoolWaveDesc &desc)
{
	desc.mPos = mpTransformComponent->getWorldPosition();
	desc.mOuterRadius = mOuterRadius;
	desc.mInnerRadius = mInnerRadius;
	desc.mDepth = mDepth;
	desc.mAngularSpeed = mAngularSpeed;
	desc.mLinearSpeed = mLinearSpeed;
	desc.mFoaminess = mFoaminess;
}

//*****************************************************************************
void VuWhirlpoolWaveEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

		VuMatrix matMVP = mpTransformComponent->getWorldTransform();
		matMVP *= params.mCamera.getViewProjMatrix();

		VuColor col(128,128,128);

		int numSides = 16;
		float fStep = 2.0f*VU_PI/numSides;
		float fCurAngle = 0.0f;
		float fNextAngle = fStep;
		for ( int iSide = 0; iSide < numSides; iSide++ )
		{
			VuVector3 v0 = VuVector3(VuCos(fCurAngle), VuSin(fCurAngle), 0);
			VuVector3 v1 = VuVector3(VuCos(fNextAngle), VuSin(fNextAngle), 0);

			VuVector3 vi0 = mInnerRadius*v0 + VuVector3(0, 0, -mDepth);
			VuVector3 vi1 = mInnerRadius*v1 + VuVector3(0, 0, -mDepth);
			VuVector3 vo0 = mOuterRadius*v0;
			VuVector3 vo1 = mOuterRadius*v1;

			pGfxUtil->drawLine3d(col, vi0, vo0, matMVP);
			pGfxUtil->drawLine3d(col, vi0, vi1, matMVP);
			pGfxUtil->drawLine3d(col, vo0, vo1, matMVP);

			fCurAngle = fNextAngle;
			fNextAngle += fStep;
		}
	}
}
