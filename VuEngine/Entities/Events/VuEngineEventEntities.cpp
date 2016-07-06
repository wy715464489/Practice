//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Events of which the engine is aware.
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Events/VuEventManager.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/HAL/Keyboard/VuKeyboard.h"


//*****************************************************************************
//
// OnProjectInitializedEntity
//
//*****************************************************************************
class VuOnProjectInitializedEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuOnProjectInitializedEntity();

private:
	void OnProjectInitialized(const VuParams &params);

	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuOnProjectInitializedEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuOnProjectInitializedEntity);

//*****************************************************************************
VuOnProjectInitializedEntity::VuOnProjectInitializedEntity()
{
	// create/add script component
	addComponent(mpScriptComponent = new VuScriptComponent(this, 120));

	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Trigger);

	// register game event handlers
	REG_EVENT_HANDLER(VuOnProjectInitializedEntity, OnProjectInitialized);
}

//*****************************************************************************
void VuOnProjectInitializedEntity::OnProjectInitialized(const VuParams &params)
{
	mpScriptComponent->getPlug("Trigger")->execute();
}


//*****************************************************************************
//
// OnProjectReleaseEntity
//
//*****************************************************************************
class VuOnProjectReleaseEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuOnProjectReleaseEntity();

private:
	void OnProjectRelease(const VuParams &params);

	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuOnProjectReleaseEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuOnProjectReleaseEntity);

//*****************************************************************************
VuOnProjectReleaseEntity::VuOnProjectReleaseEntity()
{
	// create/add script component
	addComponent(mpScriptComponent = new VuScriptComponent(this, 120));

	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Trigger);

	// register game event handlers
	REG_EVENT_HANDLER(VuOnProjectReleaseEntity, OnProjectRelease);
}

//*****************************************************************************
void VuOnProjectReleaseEntity::OnProjectRelease(const VuParams &params)
{
	mpScriptComponent->getPlug("Trigger")->execute();
}


//*****************************************************************************
//
// OnKeyboardEventEntity
//
//*****************************************************************************
class VuOnKeyboardEventEntity : public VuEntity, VuKeyboard::Callback
{
	DECLARE_RTTI

public:
	VuOnKeyboardEventEntity();

	void				onGameInitialize();
	void				onGameRelease();

private:
	// VuKeyboard::Callback
	virtual void		onKeyDown(VUUINT32 key);

	VuScriptComponent	*mpScriptComponent;

	int					mKey;
	bool				mbShift;
	bool				mbAlt;
	bool				mbDevOnly;
};

IMPLEMENT_RTTI(VuOnKeyboardEventEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuOnKeyboardEventEntity);

//*****************************************************************************
VuOnKeyboardEventEntity::VuOnKeyboardEventEntity():
	mKey(VUKEY_NONE),
	mbShift(false),
	mbAlt(false),
	mbDevOnly(true)
{
	// create/add script component
	addComponent(mpScriptComponent = new VuScriptComponent(this, 120));

	// properties
	static VuStaticIntEnumProperty::Choice sChoices[] =
	{
		{ "<none>", VUKEY_NONE },
		{ "1", VUKEY_1 }, { "2", VUKEY_2 }, { "3", VUKEY_3 }, { "4", VUKEY_4 }, { "5", VUKEY_5 },
		{ "6", VUKEY_6 }, { "7", VUKEY_7 }, { "8", VUKEY_8 }, { "9", VUKEY_9 }, { "0", VUKEY_0 },
		{ "ENTER", VUKEY_ENTER },
		{ VUNULL }
	};
	addProperty(new VuStaticIntEnumProperty("Key", mKey, sChoices));
	addProperty(new VuBoolProperty("Shift", mbShift));
	addProperty(new VuBoolProperty("Alt", mbAlt));
	addProperty(new VuBoolProperty("Dev Only", mbDevOnly));

	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, Trigger);
}

//*****************************************************************************
void VuOnKeyboardEventEntity::onGameInitialize()
{
	VuKeyboard::IF()->addCallback(this, mbDevOnly);
}

//*****************************************************************************
void VuOnKeyboardEventEntity::onGameRelease()
{
	VuKeyboard::IF()->removeCallback(this);
}

//*****************************************************************************
void VuOnKeyboardEventEntity::onKeyDown(VUUINT32 key)
{
	if ( mbShift != VuKeyboard::IF()->isKeyDown(VUKEY_SHIFT) )
		return;

	if ( mbAlt != VuKeyboard::IF()->isKeyDown(VUKEY_ALT) )
		return;

	if ( key == (VUUINT32)mKey )
		mpScriptComponent->getPlug("Trigger")->execute();
}


//*****************************************************************************
//
// VuGenericEventEntity
//
//*****************************************************************************
class VuGenericEventEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuGenericEventEntity();

	virtual void		onGameInitialize();
	virtual void		onGameRelease();

private:
	// event handlers
	void				OnGenericEvent(const VuParams &params);

	// scripting
	VuRetVal			Broadcast(const VuParams &params);

	// properties
	std::string			mEventName;

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuGenericEventEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuGenericEventEntity);

//*****************************************************************************
VuGenericEventEntity::VuGenericEventEntity()
{
	// properties
	addProperty(new VuStringProperty("Event Name", mEventName));

	// create/add script component
	addComponent(mpScriptComponent = new VuScriptComponent(this));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuGenericEventEntity, Broadcast);
	ADD_SCRIPT_OUTPUT_NOARGS(mpScriptComponent, OnReceived);
}

//*****************************************************************************
void VuGenericEventEntity::onGameInitialize()
{
	// register game event handlers
	REG_EVENT_HANDLER(VuGenericEventEntity, OnGenericEvent);
}

//*****************************************************************************
void VuGenericEventEntity::onGameRelease()
{
	mEventMap.unregisterHandlers();
}

//*****************************************************************************
VuRetVal VuGenericEventEntity::Broadcast(const VuParams &params)
{
	VuParams outParams;
	outParams.addString(mEventName.c_str());
	VuEventManager::IF()->broadcast("OnGenericEvent", outParams);

	return VuRetVal();
}

//*****************************************************************************
void VuGenericEventEntity::OnGenericEvent(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	if ( mEventName == accessor.getString() )
		mpScriptComponent->getPlug("OnReceived")->execute();
}
