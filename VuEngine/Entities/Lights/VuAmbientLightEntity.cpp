//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ambient light entity
// 
//*****************************************************************************

#include "VuAmbientLightEntity.h"
#include "VuEngine/VuEngine.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuNotifyProperty.h"
#include "VuEngine/Gfx/Light/VuLightManager.h"


IMPLEMENT_RTTI(VuAmbientLightEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAmbientLightEntity);


//*****************************************************************************
VuAmbientLightEntity::VuAmbientLightEntity():
	mDefaultLight(true),
	mColor(51,51,51),
	mFoliageColor(128,128,128)
{
	// properties
	addProperty(new VuBoolProperty("Default Light", mDefaultLight));
	VuProperty *pColorProperty = addProperty(new VuColorProperty("Color", mColor));
	VuProperty *pFoliageColorProperty = addProperty(new VuColorProperty("Foliage Color", mFoliageColor));
	addProperty(new VuNotifyProperty("Apply")) -> setWatcher(this, &VuAmbientLightEntity::apply);

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	if ( VuEngine::IF()->editorMode() )
	{
		REG_EVENT_HANDLER(VuAmbientLightEntity, OnEditorProjectSelected);
		pColorProperty->setWatcher(this, &VuAmbientLightEntity::apply);
		pFoliageColorProperty->setWatcher(this, &VuAmbientLightEntity::apply);
	}

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAmbientLightEntity, Trigger);
}

//*****************************************************************************
VuRetVal VuAmbientLightEntity::Trigger(const VuParams &params)
{
	apply();

	return VuRetVal();
}

//*****************************************************************************
void VuAmbientLightEntity::apply()
{
	VuLightManager::IF()->ambientLight().mColor = mColor;
	VuLightManager::IF()->ambientLight().mFoliageColor = mFoliageColor;
}
