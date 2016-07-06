//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  AudioEvent entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuAudioProperty.h"
#include "VuEngine/HAL/Audio/VuAudioEvent.h"


class VuAudioEventEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuAudioEventEntity();

private:
	// scripting
	VuRetVal			Start(const VuParams &params = VuParams());
	VuRetVal			Stop(const VuParams &params);

	void				modified();

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	std::string			mEventName;

	VuAudioEvent		mEvent;
};


IMPLEMENT_RTTI(VuAudioEventEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAudioEventEntity);


//*****************************************************************************
VuAudioEventEntity::VuAudioEventEntity()
{
	// properties
	addProperty(new VuAudioEventNameProperty("Event Name", mEventName))	-> setWatcher(this, &VuAudioEventEntity::modified);
	addProperty(new VuBoolProperty("Stop When Destroyed", mEvent.mbStopWhenDestroyed));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAudioEventEntity, Start);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAudioEventEntity, Stop);
}

//*****************************************************************************
VuRetVal VuAudioEventEntity::Start(const VuParams &params)
{
	if ( mEvent.create(mEventName.c_str()) )
		mEvent.start();

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuAudioEventEntity::Stop(const VuParams &params)
{
	mEvent.release();

	return VuRetVal();
}

//*****************************************************************************
void VuAudioEventEntity::modified()
{
	if ( mEvent.active() )
		Start();
}
