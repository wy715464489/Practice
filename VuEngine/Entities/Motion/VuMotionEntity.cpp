//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Motion entity
// 
//*****************************************************************************

#include "VuMotionEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Motion/VuMotionComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Json/VuJsonContainer.h"

IMPLEMENT_RTTI(VuMotionEntity, VuEntity);


//*****************************************************************************
VuMotionEntity::VuMotionEntity(VUUINT32 flags):
	VuEntity(flags),
	mbInitiallyActive(false),
	mbOneShot(false),
	mpTargetMotionComponent(VUNULL),
	mbActive(false),
	mbIsShot(false)
{
	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150));

	// properties
	addProperty(new VuBoolProperty("Initially Active", mbInitiallyActive));
	addProperty(new VuBoolProperty("One Shot", mbOneShot));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuMotionEntity, Activate);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuMotionEntity, Deactivate);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnActivated);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnDeactivated);
	mpEntityRef = ADD_SCRIPT_REF(mpScriptComponent, Entity, VuEntity::msRTTI);
}

//*****************************************************************************
void VuMotionEntity::onGameInitialize()
{
	VuTickManager::IF()->registerHandler(this, &VuMotionEntity::tickMotion, "Motion");

	if ( mpEntityRef->getRefEntity() )
		mpTargetMotionComponent = mpEntityRef->getRefEntity()->getComponent<VuMotionComponent>();

	if ( mbInitiallyActive )
		Activate();
}

//*****************************************************************************
void VuMotionEntity::onGameRelease()
{
	Deactivate();

	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
void VuMotionEntity::onGameReset()
{
	mbIsShot = false;
}

//*****************************************************************************
VuRetVal VuMotionEntity::Activate(const VuParams &params)
{
	if ( mpTargetMotionComponent )
	{
		if ( !mbIsShot && mpTargetMotionComponent->takeOwnership(this) )
		{
			mbActive = true;
			if ( mbOneShot )
				mbIsShot = true;
			onActivate();
			mpScriptComponent->getPlug("OnActivated")->execute();
		}
	}

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuMotionEntity::Deactivate(const VuParams &params)
{
	if ( mbActive )
	{
		mbActive = false;
		onDeactivate();
		mpScriptComponent->getPlug("OnDeactivated")->execute();
		mpTargetMotionComponent->relinquishOwnership(this);
	}

	return VuRetVal();
}

//*****************************************************************************
void VuMotionEntity::tickMotion(float fdt)
{
	if ( mbActive )
	{
		onUpdate(fdt);
	}
}