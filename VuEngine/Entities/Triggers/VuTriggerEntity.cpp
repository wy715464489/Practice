//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Trigger entity
// 
//*****************************************************************************

#include "VuTriggerEntity.h"
#include "VuEngine/Components/3dLayout/Vu3dLayoutComponent.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Components/Transform/VuTransformComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/Managers/VuTriggerManager.h"


IMPLEMENT_RTTI(VuTriggerEntity, VuEntity);


//*****************************************************************************
VuTriggerEntity::VuTriggerEntity():
	mbInitiallyActive(true),
	mTriggerType(VuTriggerManager::IF()->getTypes()[0]),
	mTriggerMask(1),
	mbActive(false),
	mbFirstTrigger(true)
{
	// properties
	addProperty(new VuBoolProperty("Initially Active", mbInitiallyActive));
	addProperty(new VuConstStringEnumProperty("Trigger Type", mTriggerType, VuTriggerManager::IF()->getTypes()))
		->setWatcher(this, &VuTriggerEntity::modified);

	// components
	addComponent(mp3dLayoutComponent = new Vu3dLayoutComponent(this));
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150));

	mp3dLayoutComponent->setDrawMethod(this, &VuTriggerEntity::drawLayout);

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuTriggerEntity, Activate);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuTriggerEntity, Deactivate);
	ADD_SCRIPT_OUTPUT(mpScriptComponent, OnEnter, VuRetVal::Void, VuParamDecl(1, VuParams::Entity));
	ADD_SCRIPT_OUTPUT(mpScriptComponent, OnExit, VuRetVal::Void, VuParamDecl(1, VuParams::Entity));
}

//*****************************************************************************
void VuTriggerEntity::onGameInitialize()
{
	if ( mbInitiallyActive )
		Activate();
}

//*****************************************************************************
void VuTriggerEntity::onGameRelease()
{
	Deactivate();
}

//*****************************************************************************
void VuTriggerEntity::doTrigger(VuEntity *pEntity, bool bEnter)
{
	VuParams params;
	params.addEntity(pEntity);

	if ( bEnter )
		mpScriptComponent->getPlug("OnEnter")->execute(params);
	else
		mpScriptComponent->getPlug("OnExit")->execute(params);
}

//*****************************************************************************
void VuTriggerEntity::modified()
{
	mTriggerMask = VuTriggerManager::IF()->getTypeMask(mTriggerType.c_str());
}

//*****************************************************************************
VuRetVal VuTriggerEntity::Activate(const VuParams &params)
{
	if ( !mbActive )
	{
		VuTriggerManager::IF()->addTriggerEntity(this);
		mbActive = true;
	}

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuTriggerEntity::Deactivate(const VuParams &params)
{
	if ( mbActive )
	{
		VuTriggerManager::IF()->removeTriggerEntity(this);
		mbActive = false;
	}

	return VuRetVal();
}
