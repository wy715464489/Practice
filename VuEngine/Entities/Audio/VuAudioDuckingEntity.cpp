//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  AudioDucking entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuStringProperty.h"
#include "VuEngine/HAL/Audio/VuAudio.h"


class VuAudioDuckingEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuAudioDuckingEntity();

	virtual void		onGameInitialize();
	virtual void		onGameRelease();

private:
	// scripting
	VuRetVal			Activate(const VuParams &params = VuParams());
	VuRetVal			Deactivate(const VuParams &params = VuParams());

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	bool				mbInitiallyActive;
	std::string			mCategory;
	float				mVolume;
	float				mMaxDuration;

	bool				mbActive;
	VUUINT32			mDuckingID;
};


IMPLEMENT_RTTI(VuAudioDuckingEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAudioDuckingEntity);


//*****************************************************************************
VuAudioDuckingEntity::VuAudioDuckingEntity():
	mbInitiallyActive(false),
	mVolume(-6.0f),
	mMaxDuration(10.0f),
	mbActive(false),
	mDuckingID(0)
{
	// properties
	addProperty(new VuBoolProperty("Initially Active", mbInitiallyActive));
	addProperty(new VuStringProperty("Category", mCategory));
	addProperty(new VuFloatProperty("Volume (DB)", mVolume));
	addProperty(new VuFloatProperty("Max Duration", mMaxDuration));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAudioDuckingEntity, Activate);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAudioDuckingEntity, Deactivate);
}

//*****************************************************************************
void VuAudioDuckingEntity::onGameInitialize()
{
	if ( mbInitiallyActive )
		Activate();
}

//*****************************************************************************
void VuAudioDuckingEntity::onGameRelease()
{
	Deactivate();
}

//*****************************************************************************
VuRetVal VuAudioDuckingEntity::Activate(const VuParams &params)
{
	if ( !mbActive )
	{
		mDuckingID = VuAudio::IF()->startDucking(mCategory.c_str(), mVolume, mMaxDuration);

		mbActive = true;
	}

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuAudioDuckingEntity::Deactivate(const VuParams &params)
{
	if ( mbActive )
	{
		VuAudio::IF()->stopDucking(mDuckingID);
		mbActive = false;
		mDuckingID = 0;
	}

	return VuRetVal();
}
