//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Point wave entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Properties/VuPercentageProperty.h"
#include "VuEngine/Water/VuWater.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Managers/VuTickManager.h"


class VuPointWaveEntity : public VuEntity, VuPointWaveCallbackIF, public VuMotionComponentIF
{
	DECLARE_RTTI

public:
	VuPointWaveEntity();

	virtual void			onPostLoad() { rangeModified(); }
	virtual void			onGameInitialize();
	virtual void			onGameRelease();

private:
	// scripting
	VuRetVal				Emit(const VuParams &params);

	// VuPointWaveCallbackIF
	virtual void			onPointWaveExpired();

	// VuMotionComponentIF interface
	virtual void			onMotionUpdate();

	void					tickDecision(float fdt);

	void					rangeModified();
	void					drawLayout(const Vu3dLayoutDrawParams &params);

	// components
	Vu3dLayoutComponent		*mp3dLayoutComponent;
	VuScriptComponent		*mpScriptComponent;
	VuMotionComponent		*mpMotionComponent;

	// properties
	VuWaterPointWaveDesc	mDesc;

	bool					mEmitNow;
	VuWaterPointWave		*mpWave;
};


IMPLEMENT_RTTI(VuPointWaveEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuPointWaveEntity);


//*****************************************************************************
VuPointWaveEntity::VuPointWaveEntity():
	mEmitNow(false),
	mpWave(VUNULL)
{
	// init wave desc
	mDesc.mFalloffTime = 5.0f;
	mDesc.mMagnitude = 1.0f;
	mDesc.mRangeStart = 0.0f;
	mDesc.mRangeEnd = 20.0f;
	mDesc.mSpeed = 10.0f;
	mDesc.mFrequency = 5.0f;
	mDesc.mFoaminess = 1.0f;

	// properties
	addProperty(new VuFloatProperty("Falloff Time", mDesc.mFalloffTime));
	addProperty(new VuFloatProperty("Magnitude", mDesc.mMagnitude));
	addProperty(new VuFloatProperty("Range Start", mDesc.mRangeStart));
	addProperty(new VuFloatProperty("Range End", mDesc.mRangeEnd)) -> setWatcher(this, &VuPointWaveEntity::rangeModified);
	addProperty(new VuFloatProperty("Speed", mDesc.mSpeed));
	addProperty(new VuFloatProperty("Frequency", mDesc.mFrequency));
	addProperty(new VuFloatProperty("Foaminess", mDesc.mFoaminess));

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mpScriptComponent = new VuScriptComponent(this));
	addComponent(mpMotionComponent = new VuMotionComponent(this, this));

	// limit manipulation to translation
	mpTransformComponent->setMask(VuTransformComponent::TRANS);

	mp3dLayoutComponent->setDrawMethod(this, &VuPointWaveEntity::drawLayout);
	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-1.0f, 0, 0), VuVector3(1.0f, 1.0f, 0)));

	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuPointWaveEntity, Emit);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnEmit);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnExpired);
}

//*****************************************************************************
void VuPointWaveEntity::onGameInitialize()
{
	// register phased ticks
	VuTickManager::IF()->registerHandler(this, &VuPointWaveEntity::tickDecision, "Decision");
}

//*****************************************************************************
void VuPointWaveEntity::onGameRelease()
{
	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);

	if ( mpWave )
	{
		mpWave->removeRef();

		// this is not standard... use with caution
		VUASSERT(mpWave->refCount() == 1, "VuPointWaveEntity::destroyWave() invalid ref count");
		VuWater::IF()->removeWave(mpWave);

		mpWave = VUNULL;
	}
}

//*****************************************************************************
VuRetVal VuPointWaveEntity::Emit(const VuParams &params)
{
	mEmitNow = true;

	return VuRetVal();
}

//*****************************************************************************
void VuPointWaveEntity::onPointWaveExpired()
{
	mpWave->removeRef();
	mpWave = VUNULL;

	mpScriptComponent->getPlug("OnExpired")->execute();
}

//*****************************************************************************
void VuPointWaveEntity::onMotionUpdate()
{
	mpTransformComponent->setWorldTransform(mpMotionComponent->getWorldTransform());
}

//*****************************************************************************
void VuPointWaveEntity::tickDecision(float fdt)
{
	if ( mEmitNow )
	{
		mEmitNow = false;

		// create wave
		mDesc.mPos = mpTransformComponent->getWorldPosition();
		mpWave = VuWater::IF()->createPointWave(mDesc);
		mpWave->setCallback(this);
	}
}

//*****************************************************************************
void VuPointWaveEntity::rangeModified()
{
	VuVector3 extents(mDesc.mRangeEnd, mDesc.mRangeEnd, 0);

	mp3dLayoutComponent->setLocalBounds(VuAabb(-extents, extents));
}

//*****************************************************************************
void VuPointWaveEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

		VuMatrix matMVP = mpTransformComponent->getWorldTransform();
		matMVP *= params.mCamera.getViewProjMatrix();

		// draw inner & outer radii
		if ( mDesc.mRangeStart > 0 )
			pGfxUtil->drawCylinderLines(VuColor(128,128,128), 2.0f*mDesc.mMagnitude, mDesc.mRangeStart, 32, matMVP);
		pGfxUtil->drawCylinderLines(VuColor(255,64,64), 2.0f*mDesc.mMagnitude, mDesc.mRangeEnd, 32, matMVP);
	}
}
