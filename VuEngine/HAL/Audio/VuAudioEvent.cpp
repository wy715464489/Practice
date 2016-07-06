//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Wrapper for an FMOD audio event.
// 
//*****************************************************************************

#include "VuAudioEvent.h"


#if !VU_DISABLE_AUDIO

//*****************************************************************************
static FMOD_RESULT F_CALLBACK AudioEventFmodCallback(FMOD_EVENT *event, FMOD_EVENT_CALLBACKTYPE type, void *param1, void *param2, void *userdata)
{
	return static_cast<VuAudioEvent *>(userdata)->callback(type, param1, param2, userdata);
}

//*****************************************************************************
bool VuAudioEvent::create(const char *strEventName, FMOD_EVENT_MODE mode)
{
	release();

	if ( !strEventName[0] )
		return false;

	FMOD_RESULT result = VuAudio::IF()->eventSystem()->getEvent(strEventName, mode, &mpEvent);
	if ( result == FMOD_OK )
	{
		mpEvent->setCallback(AudioEventFmodCallback, this);
	}

	return result == FMOD_OK;
}

//*****************************************************************************
void VuAudioEvent::release(eReleaseMode mode)
{
	if ( mpEvent )
	{
		mpEvent->setCallback(0, 0);
		if ( mode != NO_STOP )
			mpEvent->stop(mode == STOP_IMMEDIATE);
		mpEvent = VUNULL;
	}
}

//*****************************************************************************
FMOD_RESULT VuAudioEvent::callback(FMOD_EVENT_CALLBACKTYPE type, void *param1, void *param2, void *userdata)
{
	if ( type == FMOD_EVENT_CALLBACKTYPE_EVENTFINISHED )
	{
		mpEvent = VUNULL;
	}
	else if ( type == FMOD_EVENT_CALLBACKTYPE_SOUNDDEF_END )
	{
		if ( mbStopSoundDef )
			mpEvent->stop();
	}

	return FMOD_OK;
}

#endif // !VU_DISABLE_AUDIO