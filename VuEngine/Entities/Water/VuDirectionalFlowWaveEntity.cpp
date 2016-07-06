//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Directional flow entity
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


class VuDirectionalFlowWaveEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuDirectionalFlowWaveEntity();

	virtual void				onPostLoad() { modified(); }
	virtual void				onGameInitialize();
	virtual void				onGameRelease();

private:
	void						modified();
	void						createWaveDesc(VuWaterDirectionalFlowWaveDesc &desc);
	void						drawLayout(const Vu3dLayoutDrawParams &params);

	Vu3dLayoutComponent			*mp3dLayoutComponent;

	VuVector3					mFlowVelocity;
	float						mLongitudinalDecayRatio;
	float						mLateralDecayRatio;

	VuWaterDirectionalFlowWave	*mpWave;
};


IMPLEMENT_RTTI(VuDirectionalFlowWaveEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuDirectionalFlowWaveEntity);


//*****************************************************************************
VuDirectionalFlowWaveEntity::VuDirectionalFlowWaveEntity():
	mFlowVelocity(0,5,0),
	mLongitudinalDecayRatio(0.5),
	mLateralDecayRatio(0.5),
	mpWave(VUNULL)
{
	// properties
	addProperty(new VuVector3Property("Flow Velocity", mFlowVelocity))								->	setWatcher(this, &VuDirectionalFlowWaveEntity::modified);
	addProperty(new VuPercentageProperty("Longitudinal Decay Ratio %", mLongitudinalDecayRatio))	->	setWatcher(this, &VuDirectionalFlowWaveEntity::modified);
	addProperty(new VuPercentageProperty("Lateral Decay Ratio %", mLateralDecayRatio))				->	setWatcher(this, &VuDirectionalFlowWaveEntity::modified);

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	mp3dLayoutComponent->setDrawMethod(this, &VuDirectionalFlowWaveEntity::drawLayout);

	// want to know when transform is changed
	mpTransformComponent->setWatcher(&VuDirectionalFlowWaveEntity::modified);

	// limit manipulation to translation, rotation about Z, and scale in x-y
	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT_Z|VuTransformComponent::SCALE_X|VuTransformComponent::SCALE_Y);
}

//*****************************************************************************
void VuDirectionalFlowWaveEntity::onGameInitialize()
{
	// create wave generator
	VuWaterDirectionalFlowWaveDesc desc;
	createWaveDesc(desc);
	mpWave = VuWater::IF()->createDirectionalFlowWave(desc);
}

//*****************************************************************************
void VuDirectionalFlowWaveEntity::onGameRelease()
{
	// destroy wave generator
	mpWave->removeRef();

	// this is not standard... use with caution
	VUASSERT(mpWave->refCount() == 1, "VuDirectionalFlowWaveEntity::onGameRelease() invalid ref count");
	VuWater::IF()->removeWave(mpWave);

	mpWave = VUNULL;
}

//*****************************************************************************
void VuDirectionalFlowWaveEntity::modified()
{
	// modify wave
	if ( mpWave )
	{
		VuWaterDirectionalFlowWaveDesc desc;
		createWaveDesc(desc);
		mpWave->modify(desc);
	}

	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-0.5f, -0.5f, 0), VuVector3(0.5f, 0.5f, 0)));
}

//*****************************************************************************
void VuDirectionalFlowWaveEntity::createWaveDesc(VuWaterDirectionalFlowWaveDesc &desc)
{
	desc.mPos = mpTransformComponent->getWorldPosition();
	desc.mRotZ = mpTransformComponent->getWorldRotation().mZ;
	desc.mSizeX = mpTransformComponent->getWorldScale().mX;
	desc.mSizeY = mpTransformComponent->getWorldScale().mY;
	desc.mFlowVelocity = mFlowVelocity;
	desc.mLongitudinalDecayRatio = mLongitudinalDecayRatio;
	desc.mLateralDecayRatio = mLateralDecayRatio;
}

//*****************************************************************************
void VuDirectionalFlowWaveEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

		VuVector3 vScale = mpTransformComponent->getWorldScale();

		// draw arrows
		if ( mFlowVelocity.magSquared() > FLT_EPSILON )
		{
			VuVector3 vDir = mFlowVelocity.normal();
			float fLength = 0.5f*VuMin(vScale.mX, vScale.mY);

			VuVector3 vPos = mpTransformComponent->getWorldPosition();
			VuVector3 v0 = vPos - 0.5f*vDir*fLength;
			VuVector3 v1 = vPos + 0.5f*vDir*fLength;

			pGfxUtil->drawLine3d(v0, VuColor(64,64,64), v1, VuColor(192,192,192), params.mCamera.getViewProjMatrix());
		}

		// draw decay ratios
		{
			VuMatrix matMVP = mpTransformComponent->getWorldTransform();
			matMVP.scaleLocal(VuVector3(0.5f*vScale.mX, 0.5f*vScale.mY, 1));
			matMVP *= params.mCamera.getViewProjMatrix();

			VuVector3 verts[5];
			verts[0] = VuVector3(-mLateralDecayRatio, -mLongitudinalDecayRatio, 0);
			verts[1] = VuVector3( mLateralDecayRatio, -mLongitudinalDecayRatio, 0);
			verts[2] = VuVector3( mLateralDecayRatio,  mLongitudinalDecayRatio, 0);
			verts[3] = VuVector3(-mLateralDecayRatio,  mLongitudinalDecayRatio, 0);
			verts[4] = verts[0];

			pGfxUtil->drawLines3d(VUGFX_PT_LINESTRIP, VuColor(255,64,64), verts, 5, matMVP);
		}
	}
}
