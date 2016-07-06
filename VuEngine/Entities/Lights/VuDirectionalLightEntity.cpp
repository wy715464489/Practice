//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Directional light entity
// 
//*****************************************************************************

#include "VuDirectionalLightEntity.h"
#include "VuEngine/VuEngine.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuNotifyProperty.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"
#include "VuEngine/Gfx/Light/VuLightManager.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"


// constants

#define SHAPE_HEIGHT 10.0f
#define SHAPE_RADIUS 4.0f
#define ARROW_HEAD_LENGTH 1.0f
#define ARROW_HEAD_WIDTH 1.0f


IMPLEMENT_RTTI(VuDirectionalLightEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuDirectionalLightEntity);


//*****************************************************************************
VuDirectionalLightEntity::VuDirectionalLightEntity():
	mDefaultLight(true),
	mFrontColor(204,204,204),
	mBackColor(64,64,64),
	mSpecularColor(255,255,255),
	mFoliageColor(192,192,192)
{
	// properties
	addProperty(new VuBoolProperty("Default Light", mDefaultLight));
	VuProperty *pFrontColorProperty = addProperty(new VuColorProperty("Front Color", mFrontColor));
	VuProperty *pBackColorProperty = addProperty(new VuColorProperty("Back Color", mBackColor));
	VuProperty *pSpecularColorProperty = addProperty(new VuColorProperty("Specular Color", mSpecularColor));
	VuProperty *pFoliageColorProperty = addProperty(new VuColorProperty("Foliage Color", mFoliageColor));
	addProperty(new VuNotifyProperty("Apply")) -> setWatcher(this, &VuDirectionalLightEntity::apply);

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mpMotionComponent = new VuMotionComponent(this, this));

	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT);
	mp3dLayoutComponent->setDrawMethod(this, &VuDirectionalLightEntity::drawLayout);
	mp3dLayoutComponent->setLocalBounds(VuAabb(VuVector3(-SHAPE_RADIUS, -SHAPE_RADIUS, -SHAPE_HEIGHT), VuVector3(SHAPE_RADIUS, SHAPE_RADIUS, 0)));

	if ( VuEngine::IF()->editorMode() )
	{
		REG_EVENT_HANDLER(VuDirectionalLightEntity, OnEditorProjectSelected);
		mpTransformComponent->setWatcher(&VuDirectionalLightEntity::apply);
		pFrontColorProperty->setWatcher(this, &VuDirectionalLightEntity::apply);
		pBackColorProperty->setWatcher(this, &VuDirectionalLightEntity::apply);
		pSpecularColorProperty->setWatcher(this, &VuDirectionalLightEntity::apply);
		pFoliageColorProperty->setWatcher(this, &VuDirectionalLightEntity::apply);
	}

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuDirectionalLightEntity, Trigger);
}

//*****************************************************************************
VuRetVal VuDirectionalLightEntity::Trigger(const VuParams &params)
{
	apply();

	return VuRetVal();
}

//*****************************************************************************
void VuDirectionalLightEntity::apply()
{
	VuDirectionalLight &dirLight = VuLightManager::IF()->directionalLight();

	dirLight.mPosition = mpTransformComponent->getWorldPosition();
	dirLight.mDirection = -mpTransformComponent->getWorldTransform().getAxisZ();

	dirLight.mFrontColor = mFrontColor;
	dirLight.mBackColor = mBackColor;
	dirLight.mSpecularColor = mSpecularColor;
	dirLight.mFoliageColor = mFoliageColor;
}

//*****************************************************************************
void VuDirectionalLightEntity::drawLayout(const Vu3dLayoutDrawParams &params)
{
	VuGfxUtil *pGfxUtil = VuGfxUtil::IF();

//	VuMatrix mat = mpTransformComponent->getWorldTransform();
//	VuVector3 vPos = mat.getTrans();
//	VuVector3 vDir = -mat.getAxisZ();

	VuColor color(128, 255, 128);

	// draw center arrow
	{
		VuMatrix mat = mpTransformComponent->getWorldTransform();
		mat.rotateXLocal(VuDegreesToRadians(-90.0f));
		pGfxUtil->drawArrowLines(color, SHAPE_HEIGHT, ARROW_HEAD_LENGTH, ARROW_HEAD_WIDTH, mat*params.mCamera.getViewProjMatrix());
	}
	// draw outer arrows
	float angles[] = { 45, 135, 225, 315 };
	for ( int i = 0; i < sizeof(angles)/sizeof(angles[0]); i++ )
	{
		float fAngles = VuDegreesToRadians(angles[i]);
		VuMatrix mat = mpTransformComponent->getWorldTransform();
		mat.translateLocal(SHAPE_RADIUS*VuVector3(VuCos(fAngles), VuSin(fAngles), 0));
		mat.rotateXLocal(VuDegreesToRadians(-90.0f));
		mat.rotateYLocal(VuDegreesToRadians(-90.0f) - fAngles);
		pGfxUtil->drawArrowLines(color, SHAPE_HEIGHT, ARROW_HEAD_LENGTH, ARROW_HEAD_WIDTH, mat*params.mCamera.getViewProjMatrix());
	}
}

//*****************************************************************************
void VuDirectionalLightEntity::onMotionUpdate()
{
	VuDirectionalLight &dirLight = VuLightManager::IF()->directionalLight();

	dirLight.mPosition = mpMotionComponent->getWorldTransform().getTrans();
	dirLight.mDirection = -mpMotionComponent->getWorldTransform().getAxisZ();
}