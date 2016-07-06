//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dynamic light entity
// 
//*****************************************************************************

#include "VuDynamicLightEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuAngleProperty.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/Light/VuLightManager.h"


IMPLEMENT_RTTI(VuDynamicLightEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuDynamicLightEntity);


//*****************************************************************************
VuDynamicLightEntity::VuDynamicLightEntity():
	mbInitiallyOn(false)
{
	// properties
	addProperty(new VuBoolProperty("Initially On", mbInitiallyOn));
	addProperty(new VuColorProperty("Diffuse Color", mDynamicLight.mDiffuseColor))			-> setWatcher(this, &VuDynamicLightEntity::modified);
	addProperty(new VuColorProperty("Specular Color", mDynamicLight.mSpecularColor))		-> setWatcher(this, &VuDynamicLightEntity::modified);
	addProperty(new VuFloatProperty("Factor", mDynamicLight.mFactor))						-> setWatcher(this, &VuDynamicLightEntity::modified);
	addProperty(new VuFloatProperty("Falloff Range Min", mDynamicLight.mFalloffRangeMin))	-> setWatcher(this, &VuDynamicLightEntity::modified);
	addProperty(new VuFloatProperty("Falloff Range Max", mDynamicLight.mFalloffRangeMax))	-> setWatcher(this, &VuDynamicLightEntity::modified);
	addProperty(new VuAngleProperty("Cone Angle", mDynamicLight.mConeAngle))				-> setWatcher(this, &VuDynamicLightEntity::modified);
	addProperty(new VuAngleProperty("Penumbra Angle", mDynamicLight.mPenumbraAngle))		-> setWatcher(this, &VuDynamicLightEntity::modified);
	addProperty(new VuFloatProperty("Draw Distance", mDynamicLight.mDrawDistance));
	addProperty(new VuBoolProperty("Reflecting", mDynamicLight.mbReflecting));

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150, false));
	addComponent(mpMotionComponent = new VuMotionComponent(this, this));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuDynamicLightEntity, TurnOn);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuDynamicLightEntity, TurnOff);

	mpTransformComponent->setWatcher(&VuDynamicLightEntity::modified);
	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT);
	mp3dLayoutComponent->setDrawMethod(this, &VuDynamicLightEntity::drawLayout);
}

//*****************************************************************************
void VuDynamicLightEntity::onGameInitialize()
{
	if ( mbInitiallyOn )
		mDynamicLight.turnOn();
}

//*****************************************************************************
void VuDynamicLightEntity::onGameRelease()
{
	mDynamicLight.turnOff();
}

//*****************************************************************************
void VuDynamicLightEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	if ( params.mbSelected )
	{
		mDynamicLight.getRenderLight().debugDraw(params.mCamera.getViewProjMatrix());
	}
}

//*****************************************************************************
void VuDynamicLightEntity::modified()
{
	const VuMatrix &mat = mpTransformComponent->getWorldTransform();
	mDynamicLight.mPosition = mat.getTrans();
	mDynamicLight.mDirection = mat.getAxisY();

	mDynamicLight.update();
}

//*****************************************************************************
void VuDynamicLightEntity::onMotionUpdate()
{
	mpTransformComponent->setWorldTransform(mpMotionComponent->getWorldTransform());
}
