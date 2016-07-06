//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Audio Entities
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/HAL/Audio/VuAudio.h"


//*****************************************************************************
// Dolby
//*****************************************************************************
class VuDolbyAudioEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuDolbyAudioEntity();

private:
	VuRetVal IsSupported(const VuParams &params) { return VuRetVal(VuAudio::IF()->isDolbyAudioProcessingSupported()); }
	VuRetVal IsEnabled(const VuParams &params) { return VuRetVal(VuAudio::IF()->isDolbyAudioProcessingEnabled()); }
	VuRetVal Enable(const VuParams &params) { VuAudio::IF()->setDolbyAudioProcessingEnabled(true); return VuRetVal(); }
	VuRetVal Disable(const VuParams &params) { VuAudio::IF()->setDolbyAudioProcessingEnabled(false); return VuRetVal(); }

	// components
	VuScriptComponent	*mpScriptComponent;
};

IMPLEMENT_RTTI(VuDolbyAudioEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuDolbyAudioEntity);


//*****************************************************************************
VuDolbyAudioEntity::VuDolbyAudioEntity()
{
	addComponent(mpScriptComponent = new VuScriptComponent(this));

	ADD_SCRIPT_INPUT(mpScriptComponent, VuDolbyAudioEntity, IsSupported, VuRetVal::Bool, VuParamDecl());
	ADD_SCRIPT_INPUT(mpScriptComponent, VuDolbyAudioEntity, IsEnabled, VuRetVal::Bool, VuParamDecl());
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuDolbyAudioEntity, Enable);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuDolbyAudioEntity, Disable);
}
