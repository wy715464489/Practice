//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Wrapper for an FMOD audio event.
// 
//*****************************************************************************

#pragma once

#include "VuAudio.h"


#if VU_DISABLE_AUDIO

class VuAudioEvent
{
public:
	VuAudioEvent() : mbStopWhenDestroyed(true), mbStopSoundDef(false) {}

	enum eReleaseMode { NO_STOP, STOP, STOP_IMMEDIATE };

	bool			create(const char *strEventName, FMOD_EVENT_MODE mode = FMOD_EVENT_NONBLOCKING) { return false; }
	void			release(eReleaseMode mode = STOP) {}

	bool			active() { return false; }

	inline void		start() {}
	inline void		set3DAttributes(const VuVector3 *position, const VuVector3 *velocity, const VuVector3 *orientation) {}
	inline void		setParameterValue(const char *name, float value) {}
	inline void		setVolume(float volume) {}

	// stop when destroyed?
	bool			mbStopWhenDestroyed;
	bool			mbStopSoundDef;
};

#else // VU_DISABLE_AUDIO

class VuAudioEvent
{
public:
	VuAudioEvent() : mbStopWhenDestroyed(true), mbStopSoundDef(false), mpEvent(VUNULL) {}
	~VuAudioEvent() { release(mbStopWhenDestroyed ? STOP_IMMEDIATE : NO_STOP); }

	enum eReleaseMode { NO_STOP, STOP, STOP_IMMEDIATE };

	bool			create(const char *strEventName, FMOD_EVENT_MODE mode = FMOD_EVENT_NONBLOCKING);
	void			release(eReleaseMode mode = STOP);

	bool			active() { return mpEvent ? true : false; }

	inline void		start();
	inline void		set3DAttributes(const VuVector3 *position, const VuVector3 *velocity, const VuVector3 *orientation);
	inline void		setParameterValue(const char *name, float value);
	inline void		setVolume(float volume);

	// callback
	FMOD_RESULT		callback(FMOD_EVENT_CALLBACKTYPE type, void *param1, void *param2, void *userdata);

	// stop when destroyed?
	bool			mbStopWhenDestroyed;
	bool			mbStopSoundDef;

private:
	FMOD::Event		*mpEvent;
};

//*****************************************************************************
inline void VuAudioEvent::start()
{
	mpEvent->start();
}

//*****************************************************************************
inline void VuAudioEvent::set3DAttributes(const VuVector3 *position, const VuVector3 *velocity, const VuVector3 *orientation)
{
	mpEvent->set3DAttributes(reinterpret_cast<const FMOD_VECTOR *>(position), reinterpret_cast<const FMOD_VECTOR *>(velocity), reinterpret_cast<const FMOD_VECTOR *>(orientation));
}

//*****************************************************************************
inline void VuAudioEvent::setParameterValue(const char *name, float value)
{
	FMOD::EventParameter *pParam;
	if ( mpEvent->getParameter(name, &pParam) == FMOD_OK )
		pParam->setValue(value);
}

//*****************************************************************************
inline void VuAudioEvent::setVolume(float volume)
{
	mpEvent->setVolume(volume);
}

#endif // VU_DISABLE_AUDIO
