//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Lens Water Emitter entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Managers/VuLensWaterManager.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"


class VuLensWaterEmitterEntity : public VuEntity, public VuLensWaterManager::VuEmitterIF
{
	DECLARE_RTTI

public:
	VuLensWaterEmitterEntity();

	void				onGameInitialize();
	void				onGameRelease();

private:
	// scripting
	VuRetVal			Activate(const VuParams &params) { activate(); return VuRetVal(); }
	VuRetVal			Deactivate(const VuParams &params) { deactivate(); return VuRetVal(); }

	void				activate();
	void				deactivate();

	// VuLensWaterManager::VuEmitterIF
	virtual float		lensWaterRate(const VuVector3 &pos);

	void				drawLayout(const Vu3dLayoutDrawParams &params);

	// properties
	bool				mInitiallyActive;
	float				mRadius;
	float				mDropsPerSecond;

	// components
	Vu3dLayoutComponent	*mp3dLayoutComponent;
	VuScriptComponent	*mpScriptComponent;

	float				mActive;
};


IMPLEMENT_RTTI(VuLensWaterEmitterEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuLensWaterEmitterEntity);


//*****************************************************************************
VuLensWaterEmitterEntity::VuLensWaterEmitterEntity():
	mInitiallyActive(true),
	mRadius(25.0f),
	mDropsPerSecond(64.0f),
	mActive(false)
{
	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150, false));

	mp3dLayoutComponent->setDrawMethod(this, &VuLensWaterEmitterEntity::drawLayout);

	// properties
	addProperty(new VuBoolProperty("Initially Active", mInitiallyActive));
	addProperty(new VuFloatProperty("Radius", mRadius));
	addProperty(new VuFloatProperty("Drops Per Second", mDropsPerSecond));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuLensWaterEmitterEntity, Activate);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuLensWaterEmitterEntity, Deactivate);
}

//*****************************************************************************
void VuLensWaterEmitterEntity::onGameInitialize()
{
	if ( mInitiallyActive )
		activate();
}

//*****************************************************************************
void VuLensWaterEmitterEntity::onGameRelease()
{
	deactivate();
	VuLensWaterManager::IF()->unregisterEmitter(this);
}

//*****************************************************************************
void VuLensWaterEmitterEntity::activate()
{
	if ( !mActive )
	{
		VuLensWaterManager::IF()->registerEmitter(this);
		mActive = true;
	}
}

//*****************************************************************************
void VuLensWaterEmitterEntity::deactivate()
{
	if ( mActive )
	{
		VuLensWaterManager::IF()->unregisterEmitter(this);
		mActive = false;
	}
}

//*****************************************************************************
float VuLensWaterEmitterEntity::lensWaterRate(const VuVector3 &pos)
{
	VuVector3 diff = mpTransformComponent->getWorldPosition() - pos;

	if ( diff.magSquared() <= mRadius*mRadius )
		return mDropsPerSecond;

	return 0.0f;
}

//*****************************************************************************
void VuLensWaterEmitterEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		VuMatrix matMVP = mpTransformComponent->getWorldTransform()*params.mCamera.getViewProjMatrix();

		VuGfxUtil::IF()->drawSphereLines(VuColor(255, 255, 0), mRadius, 8, 8, matMVP);
	}
}
