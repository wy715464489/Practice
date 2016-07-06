//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Audio utility functionality.
// 
//*****************************************************************************

#include "VuAudioUtil.h"
#include "VuEngine/HAL/Audio/VuAudio.h"


//*****************************************************************************
void VuAudioUtil::playSfx(const char *sfx)
{
#if !VU_DISABLE_AUDIO
	FMOD::Event *pEvent;
	if ( VuAudio::IF()->eventSystem()->getEvent(sfx, FMOD_EVENT_NONBLOCKING, &pEvent) == FMOD_OK )
		pEvent->start();
#endif
}

//*****************************************************************************
void VuAudioUtil::playSfx(const char *sfx, const VuVector3 &pos)
{
#if !VU_DISABLE_AUDIO
	FMOD::Event *pEvent;
	if ( VuAudio::IF()->eventSystem()->getEvent(sfx, FMOD_EVENT_NONBLOCKING, &pEvent) == FMOD_OK )
	{
		FMOD_VECTOR fmodPos = VuAudio::toFmodVector(pos);
		pEvent->set3DAttributes(&fmodPos, VUNULL, VUNULL);
		pEvent->start();
	}
#endif
}
