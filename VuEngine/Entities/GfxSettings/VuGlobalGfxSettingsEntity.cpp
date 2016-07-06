//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  GlobalGfxSettings entity
// 
//*****************************************************************************

#include "VuGfxSettingsEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuNotifyProperty.h"
#include "VuEngine/Managers/VuGfxSettingsManager.h"


class VuGlobalGfxSettingsEntity : public VuGfxSettingsEntity
{
	DECLARE_RTTI

public:
	VuGlobalGfxSettingsEntity();

private:
	virtual VuRetVal	Trigger(const VuParams &params)	{ use(); return VuRetVal(); }

	void				use();
};


IMPLEMENT_RTTI(VuGlobalGfxSettingsEntity, VuGfxSettingsEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuGlobalGfxSettingsEntity);


//*****************************************************************************
VuGlobalGfxSettingsEntity::VuGlobalGfxSettingsEntity()
{
	addProperty(new VuNotifyProperty("Use")) -> setWatcher(this, &VuGlobalGfxSettingsEntity::use);
}

//*****************************************************************************
void VuGlobalGfxSettingsEntity::use()
{
	VuGfxSettingsManager::IF()->setGlobal(mSettings);
}