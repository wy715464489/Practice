//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Locator entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"

class VuLocatorEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuLocatorEntity();

private:
	// components
	VuScriptComponent* mpScriptComponent;
};


IMPLEMENT_RTTI(VuLocatorEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuLocatorEntity);

//*****************************************************************************
VuLocatorEntity::VuLocatorEntity()
{
	addComponent(mpScriptComponent = new VuScriptComponent(this));
}
