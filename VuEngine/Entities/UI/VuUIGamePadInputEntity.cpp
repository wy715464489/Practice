//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Game Pad Input classes
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/UI/VuUIInputUtil.h"


class VuUIGamePadInputEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuUIGamePadInputEntity();

private:
	void				OnUIGamePad(const VuParams &params);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	int					mChannel;
	bool				mDown;
};

IMPLEMENT_RTTI(VuUIGamePadInputEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuUIGamePadInputEntity);


// choices for size
static VuStaticIntEnumProperty::Choice sChannelChoices[] =
{
	{ "Select", VuUIInputUtil::CHANNEL_SELECT },
	{ "Back",   VuUIInputUtil::CHANNEL_BACK },
	{ "Up",     VuUIInputUtil::CHANNEL_UP },
	{ "Down",   VuUIInputUtil::CHANNEL_DOWN },
	{ "Left",   VuUIInputUtil::CHANNEL_LEFT },
	{ "Right",  VuUIInputUtil::CHANNEL_RIGHT },
	{ VUNULL }
};


//*****************************************************************************
VuUIGamePadInputEntity::VuUIGamePadInputEntity():
	mChannel(VuUIInputUtil::CHANNEL_SELECT),
	mDown(true)
{
	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 150));

	// scripting
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Trigger);

	// properties
	addProperty(new VuStaticIntEnumProperty("Channel", mChannel, sChannelChoices));
	addProperty(new VuBoolProperty("Down", mDown));

	// event handlers
	REG_EVENT_HANDLER(VuUIGamePadInputEntity, OnUIGamePad);
}

//*****************************************************************************
void VuUIGamePadInputEntity::OnUIGamePad(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	int channel = accessor.getInt();
	bool down = accessor.getBool();
	int padIndex = accessor.getInt();

	if ( channel == mChannel && down == mDown )
		mpScriptComponent->getPlug("Trigger")->execute();
}
