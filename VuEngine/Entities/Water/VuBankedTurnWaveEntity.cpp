//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Banked turn entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Properties/VuAngleProperty.h"
#include "VuEngine/Water/VuWater.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/Math/VuMatrix.h"


class VuBankedTurnWaveEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuBankedTurnWaveEntity();

	virtual void			onPostLoad() { modified(); }
	virtual void			onGameInitialize();
	virtual void			onGameRelease();

private:
	void					modified();
	void					createWaveDesc(VuWaterBankedTurnWaveDesc &desc);
	void					drawLayout(const Vu3dLayoutDrawParams &params);

	Vu3dLayoutComponent		*mp3dLayoutComponent;

	float					mOuterRadius;
	float					mInnerRadius;
	float					mHeight;
	float					mAngularSize;
	float					mAngularDecayRatio;

	VuWaterBankedTurnWave	*mpWave;
};


IMPLEMENT_RTTI(VuBankedTurnWaveEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuBankedTurnWaveEntity);


//*****************************************************************************
VuBankedTurnWaveEntity::VuBankedTurnWaveEntity():
	mOuterRadius(20),
	mInnerRadius(10),
	mHeight(2),
	mAngularSize(VU_2PI),
	mAngularDecayRatio(0.5),
	mpWave(VUNULL)
{
	// properties
	addProperty(new VuFloatProperty("Inner Radius", mInnerRadius))				->	setWatcher(this, &VuBankedTurnWaveEntity::modified);
	addProperty(new VuFloatProperty("Outer Radius", mOuterRadius))				->	setWatcher(this, &VuBankedTurnWaveEntity::modified);
	addProperty(new VuFloatProperty("Height", mHeight))							->	setWatcher(this, &VuBankedTurnWaveEntity::modified);
	addProperty(new VuAngleProperty("Angular Size", mAngularSize))				->	setWatcher(this, &VuBankedTurnWaveEntity::modified);
	addProperty(new VuFloatProperty("Angular Decay Ratio", mAngularDecayRatio))	->	setWatcher(this, &VuBankedTurnWaveEntity::modified);

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	mp3dLayoutComponent->setDrawMethod(this, &VuBankedTurnWaveEntity::drawLayout);

	// want to know when transform is changed
	mpTransformComponent->setWatcher(&VuBankedTurnWaveEntity::modified);

	// limit manipulation to translation & rotation about z
	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT_Z);
	modified();
}

//*****************************************************************************
void VuBankedTurnWaveEntity::onGameInitialize()
{
	// create wave generator
	VuWaterBankedTurnWaveDesc desc;
	createWaveDesc(desc);
	mpWave = VuWater::IF()->createBankedTurnWave(desc);
}

//*****************************************************************************
void VuBankedTurnWaveEntity::onGameRelease()
{
	// destroy wave generator
	mpWave->removeRef();

	// this is not standard... use with caution
	VUASSERT(mpWave->refCount() == 1, "VuBankedTurnWaveEntity::onGameRelease() invalid ref count");
	VuWater::IF()->removeWave(mpWave);

	mpWave = VUNULL;
}

//*****************************************************************************
void VuBankedTurnWaveEntity::modified()
{
	// modify wave
	if ( mpWave )
	{
		VuWaterBankedTurnWaveDesc desc;
		createWaveDesc(desc);
		mpWave->modify(desc);
	}

	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-mOuterRadius, -mOuterRadius, 0), VuVector3(mOuterRadius, mOuterRadius, mHeight)));
}

//*****************************************************************************
void VuBankedTurnWaveEntity::createWaveDesc(VuWaterBankedTurnWaveDesc &desc)
{
	desc.mPos = mpTransformComponent->getWorldPosition();
	desc.mRotZ = mpTransformComponent->getWorldRotation().mZ;
	desc.mOuterRadius = mOuterRadius;
	desc.mInnerRadius = mInnerRadius;
	desc.mHeight = mHeight;
	desc.mAngularSize = mAngularSize;
	desc.mAngularDecayRatio = mAngularDecayRatio;
}

//*****************************************************************************
void VuBankedTurnWaveEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

		VuMatrix matMVP = mpTransformComponent->getWorldTransform();
		matMVP *= params.mCamera.getViewProjMatrix();

		VuColor col(128,128,128);

		int numSegments = 16;
		for ( int iSide = 0; iSide <= numSegments; iSide++ )
		{
			float fCurAngle = -0.5f*mAngularSize + mAngularSize*iSide/numSegments;
			float fNextAngle = -0.5f*mAngularSize + mAngularSize*(iSide + 1)/numSegments;
			VuVector3 v0 = VuVector3(-VuSin(fCurAngle), VuCos(fCurAngle), 0);
			VuVector3 v1 = VuVector3(-VuSin(fNextAngle), VuCos(fNextAngle), 0);

			VuVector3 vi0 = mInnerRadius*v0;
			VuVector3 vi1 = mInnerRadius*v1;
			VuVector3 vo0 = mOuterRadius*v0;
			VuVector3 vo1 = mOuterRadius*v1;
			VuVector3 vc0 = 0.5f*(vi0 + vo0) + VuVector3(0, 0, mHeight);
			VuVector3 vc1 = 0.5f*(vi1 + vo1) + VuVector3(0, 0, mHeight);

			pGfxUtil->drawLine3d(col, vi0, vc0, matMVP);
			pGfxUtil->drawLine3d(col, vc0, vo0, matMVP);

			if ( iSide < numSegments )
			{
				pGfxUtil->drawLine3d(col, vi0, vi1, matMVP);
				pGfxUtil->drawLine3d(col, vc0, vc1, matMVP);
				pGfxUtil->drawLine3d(col, vo0, vo1, matMVP);
			}
		}
	}
}
