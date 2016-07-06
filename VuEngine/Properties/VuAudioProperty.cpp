//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Audio property classes
// 
//*****************************************************************************

#include "VuAudioProperty.h"
#include "VuEngine/HAL/Audio/VuAudio.h"


//*****************************************************************************
int VuAudioEventNameProperty::getChoiceCount() const
{
	return VuAudio::IF()->getInfo()["Events"].size();
}

//*****************************************************************************
const char *VuAudioEventNameProperty::getChoice(int index) const
{
	return VuAudio::IF()->getInfo()["Events"][index].asCString();
}

//*****************************************************************************
int VuAudioReverbNameProperty::getChoiceCount() const
{
	return VuAudio::IF()->getInfo()["Reverbs"].size();
}

//*****************************************************************************
const char *VuAudioReverbNameProperty::getChoice(int index) const
{
	return VuAudio::IF()->getInfo()["Reverbs"][index].asCString();
}
