//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  One-Shot Pfx entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuStringProperty.h"
#include "VuEngine/Pfx/VuPfx.h"
#include "VuEngine/Pfx/VuPfxManager.h"
#include "VuEngine/Pfx/VuPfxEntity.h"


class VuOneShotPfxEntity : public VuEntity, public VuMotionComponentIF
{
	DECLARE_RTTI

public:
	VuOneShotPfxEntity();

private:
	// scripting
	VuRetVal			Start(const VuParams &params = VuParams());
	VuRetVal			Stop(const VuParams &params = VuParams());
	VuRetVal			Kill(const VuParams &params = VuParams());

	// VuMotionComponentIF interface
	virtual void		onMotionUpdate();

	// components
	VuScriptComponent		*mpScriptComponent;
	VuMotionComponent		*mpMotionComponent;

	// properties
	std::string			mPfxSystemName;
	float				mPfxScale;
	VuColor				mPfxColor;
	bool				mEnableReflection;
	bool				mEnableShadow;

	VUUINT32			mhPfx;
};


IMPLEMENT_RTTI(VuOneShotPfxEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuOneShotPfxEntity);


//*****************************************************************************
VuOneShotPfxEntity::VuOneShotPfxEntity():
	mPfxScale(1.0f),
	mPfxColor(255,255,255,255),
	mEnableReflection(false),
	mEnableShadow(false),
	mhPfx(0)
{
	// properties
	addProperty(new VuStringProperty("Effect Name", mPfxSystemName));
	addProperty(new VuFloatProperty("Pfx Scale", mPfxScale));
	addProperty(new VuColorProperty("Pfx Color", mPfxColor));
	addProperty(new VuBoolProperty("Enable Reflection", mEnableReflection));
	addProperty(new VuBoolProperty("Enable Shadow", mEnableShadow));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150, false));
	addComponent(mpMotionComponent = new VuMotionComponent(this, this));

	// want to know when transform is changed
	mpTransformComponent->setMask(VuTransformComponent::TRANS|VuTransformComponent::ROT);

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuOneShotPfxEntity, Start);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuOneShotPfxEntity, Stop);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuOneShotPfxEntity, Kill);
}

//*****************************************************************************
VuRetVal VuOneShotPfxEntity::Start(const VuParams &params)
{
	// create effect
	if ( (mhPfx = VuPfxManager::IF()->createEntity(mPfxSystemName.c_str(), true)) != VUNULL)
	{
		if ( VuPfxEntity *pPfxEntity = VuPfxManager::IF()->getEntity(mhPfx) )
		{
			pPfxEntity->enableReflection(mEnableReflection);
			pPfxEntity->enableShadow(mEnableShadow);

			pPfxEntity->getSystemInstance()->setScale(mPfxScale);
			pPfxEntity->getSystemInstance()->setColor(mPfxColor.toVector4());
			pPfxEntity->getSystemInstance()->setMatrix(mpTransformComponent->getWorldTransform());

			pPfxEntity->getSystemInstance()->start();
		}
	}

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuOneShotPfxEntity::Stop(const VuParams &params)
{
	if ( VuPfxEntity *pPfxEntity = VuPfxManager::IF()->getEntity(mhPfx) )
		pPfxEntity->getSystemInstance()->stop(false);

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuOneShotPfxEntity::Kill(const VuParams &params)
{
	if ( VuPfxEntity *pPfxEntity = VuPfxManager::IF()->getEntity(mhPfx) )
		pPfxEntity->getSystemInstance()->stop(true);

	return VuRetVal();
}

//*****************************************************************************
void VuOneShotPfxEntity::onMotionUpdate()
{
	mpTransformComponent->setWorldTransform(mpMotionComponent->getWorldTransform(), false);

	if ( VuPfxEntity *pPfxEntity = VuPfxManager::IF()->getEntity(mhPfx) )
		pPfxEntity->getSystemInstance()->setMatrix(mpMotionComponent->getWorldTransform());
}