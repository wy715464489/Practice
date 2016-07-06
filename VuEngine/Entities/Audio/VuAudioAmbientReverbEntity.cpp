//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  AudioAmbientReverb entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuAudioProperty.h"
#include "VuEngine/HAL/Audio/VuAudioEvent.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/Camera/VuCamera.h"

#if VU_DISABLE_AUDIO

class VuAudioAmbientReverbEntity : public VuEntity
{
	DECLARE_RTTI
};
IMPLEMENT_RTTI(VuAudioAmbientReverbEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAudioAmbientReverbEntity);

#else // VU_DISABLE_AUDIO

class VuAudioAmbientReverbEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuAudioAmbientReverbEntity();

	virtual void		onGameInitialize();

private:
	// event handlers
	void				OnReverbSettingChanged(const VuParams &params);

	// scripting
	VuRetVal			Activate(const VuParams &params = VuParams());
	VuRetVal			Deactivate(const VuParams &params = VuParams());

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	std::string			mReverbName;
	bool				mbInitiallyActive;

	bool				mActive;
};


IMPLEMENT_RTTI(VuAudioAmbientReverbEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAudioAmbientReverbEntity);


//*****************************************************************************
VuAudioAmbientReverbEntity::VuAudioAmbientReverbEntity():
	mbInitiallyActive(true),
	mActive(false)
{
	// event handlers
	REG_EVENT_HANDLER(VuAudioAmbientReverbEntity, OnReverbSettingChanged);

	// properties
	addProperty(new VuAudioReverbNameProperty("Reverb Name", mReverbName));
	addProperty(new VuBoolProperty("Initially Active", mbInitiallyActive));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100, false));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAudioAmbientReverbEntity, Activate);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAudioAmbientReverbEntity, Deactivate);
}

//*****************************************************************************
void VuAudioAmbientReverbEntity::onGameInitialize()
{
	if ( mbInitiallyActive )
		Activate();
	else
		Deactivate();
}

//*****************************************************************************
void VuAudioAmbientReverbEntity::OnReverbSettingChanged(const VuParams &params)
{
	if ( mActive )
	{
		Deactivate();
		Activate();
	}
}

//*****************************************************************************
VuRetVal VuAudioAmbientReverbEntity::Activate(const VuParams &params)
{
	mActive = true;

	if ( mReverbName.length() )
	{
		FMOD_REVERB_PROPERTIES props = FMOD_PRESET_OFF;
		VuAudio::IF()->getReverbPreset(mReverbName, props);
		FMODCALL(VuAudio::IF()->eventSystem()->setReverbAmbientProperties(&props));
	}

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuAudioAmbientReverbEntity::Deactivate(const VuParams &params)
{
	mActive = false;

	FMOD_REVERB_PROPERTIES props = FMOD_PRESET_OFF;
	FMODCALL(VuAudio::IF()->eventSystem()->setReverbAmbientProperties(&props));

	return VuRetVal();
}

#endif // VU_DISABLE_AUDIO
