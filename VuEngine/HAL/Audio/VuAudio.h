//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class for Audio.
// 
//*****************************************************************************

#pragma once

#if defined VUWIN32
	#include "FMOD/Win32/4.44.26/api/inc/fmod.hpp"
	#include "FMOD/Win32/4.44.26/api/inc/fmod_errors.h"
	#include "FMOD/Win32/4.44.26/dapi/inc/fmod_event.hpp"
#elif defined VUWINSTORE
	#include "FMOD/WinStore/4.44.46/api/inc/fmod.hpp"
	#include "FMOD/WinStore/4.44.46/api/inc/fmod_errors.h"
	#include "FMOD/WinStore/4.44.46/dapi/inc/fmod_event.hpp"
#elif defined VUWINPHONE
	#include "FMOD/WinPhone/4.44.46/api/inc/fmod.hpp"
	#include "FMOD/WinPhone/4.44.46/api/inc/fmod_errors.h"
	#include "FMOD/WinPhone/4.44.46/dapi/inc/fmod_event.hpp"
#elif defined VUANDROID
	#include "FMOD/Android/4.44.26/api/inc/fmod.hpp"
	#include "FMOD/Android/4.44.26/api/inc/fmod_errors.h"
	#include "FMOD/Android/4.44.26/dapi/inc/fmod_event.hpp"
#elif defined VUIOS
	#include "FMOD/Ios/4.44.26/api/inc/fmod.hpp"
	#include "FMOD/Ios/4.44.26/api/inc/fmod_errors.h"
	#include "FMOD/Ios/4.44.26/api/inc/fmodiphone.h"
	#include "FMOD/Ios/4.44.26/dapi/inc/fmod_event.hpp"
#elif defined VUPS4
	#include "FMOD/Ps4/4.44.49/api/inc/fmod.hpp"
	#include "FMOD/Ps4/4.44.49/api/inc/fmod_errors.h"
	#include "FMOD/Ps4/4.44.49/dapi/inc/fmod_event.hpp"
#elif defined VUXB1
	#include "FMOD/Xb1/4.44.52/api/inc/fmod.hpp"
	#include "FMOD/Xb1/4.44.52/api/inc/fmod_errors.h"
	#include "FMOD/Xb1/4.44.52/dapi/inc/fmod_event.hpp"
#endif

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Containers/VuObjectArray.h"
#include "VuEngine/Json/VuJsonContainer.h"

class VuEngine;


#if VU_DISABLE_AUDIO

class VuAudio : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuAudio)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init() { return true; }

public:
	FMOD::EventSystem *			eventSystem()	{ return VUNULL; }
	FMOD::System *				system()		{ return VUNULL; }
	FMOD::MusicSystem *			musicSystem()	{ return VUNULL; }

	FMOD::EventCategory *		getMasterCategory()	{ return VUNULL; }
	FMOD::EventCategory *		getMusicCategory()	{ return VUNULL; }

	bool						getMinMaxDist(FMOD::Event *pEvent, float &minDist, float &maxDist) { return false; }

	// listeners
	void						setListenerCount(int count) {}
	void						setListenerAttributes(int index, const VuVector3 &pos, const VuVector3 &vel, const VuVector3 &fwd, const VuVector3 &up) {}

	VUUINT32					startDucking(const char *strCategory, float volume, float maxDuration) { return 0; }
	void						stopDucking(VUUINT32 id) {}

	void						enumerateRevertPresets() {}
	bool						getReverbPreset(const std::string &strName, FMOD_REVERB_PROPERTIES &props) { return false; }

	const char					*getEncryptionKey() { return ""; }

	const VuJsonContainer		&getInfo()		{ return VuJsonContainer::null; }

	static inline FMOD_VECTOR	toFmodVector(const VuVector3 &vec);
	static inline VuVector3		toVuVector3(const FMOD_VECTOR &vec);

	static bool					isOtherAudioPlaying() { return false; }

	void						pushPauseRequest(const char *category) {}
	void						popPauseRequest(const char *category) {}

	void						pushMasterPause() {}
	void						popMasterPause() {}
};

#else // VU_DISABLE_AUDIO

class VuAudio : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuAudio)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init();
	virtual void release();
	virtual void postInit();

public:
	VuAudio();

	FMOD::EventSystem *			eventSystem()	{ return mpEventSystem; }
	FMOD::System *				system()		{ return mpSystem; }
	FMOD::MusicSystem *			musicSystem()	{ return mpMusicSystem; }

	FMOD::EventCategory *		getMasterCategory()	{ return mpMasterCategory; }
	FMOD::EventCategory *		getMusicCategory()	{ return mpMusicCategory; }

	bool						getMinMaxDist(FMOD::Event *pEvent, float &minDist, float &maxDist);

	// listeners
	void						setListenerCount(int count);
	void						setListenerAttributes(int index, const VuVector3 &pos, const VuVector3 &vel, const VuVector3 &fwd, const VuVector3 &up);

	VUUINT32					startDucking(const char *strCategory, float volume, float maxDuration);
	void						stopDucking(VUUINT32 id);

	void						enumerateRevertPresets();
	bool						getReverbPreset(const std::string &strName, FMOD_REVERB_PROPERTIES &props);

	const char					*getEncryptionKey() { return mstrEncryptionKey; }

	const VuJsonContainer		&getInfo()		{ return mInfo; }

	static inline FMOD_VECTOR	toFmodVector(const VuVector3 &vec);
	static inline VuVector3		toVuVector3(const FMOD_VECTOR &vec);

	static bool					isOtherAudioPlaying();

	void						pushCategoryPause(const char *category);
	void						popCategoryPause(const char *category);

	void						pushMasterPause();
	void						popMasterPause();

	// dolby
	virtual bool				isDolbyAudioProcessingSupported() { return false; }
	virtual bool				isDolbyAudioProcessingEnabled() { return false; }
	virtual void				setDolbyAudioProcessingEnabled(bool enable) { }

private:
	void						tickAudio(float fdt);
	void						updateDucking(float fdt);
	void						updateDevStats();
	void						drawEmitters();
	void						loadInfo();
	void						parseInfo(const std::string &projectName, const char *strText, VuJsonContainer &info);

	struct VuDuckingEntry
	{
		float				calcVolume();

		VUUINT32			mID;
		float				mAge;
		float				mDuration;
		float				mVolume;
	};
	typedef VuObjectArray<VuDuckingEntry> DuckingEntries;
	typedef std::map<FMOD::EventCategory *, DuckingEntries> DuckingCategories;
	typedef std::map<std::string, FMOD_REVERB_PROPERTIES> ReverbPresets;

	const char					*mstrEncryptionKey;
	FMOD::EventSystem *			mpEventSystem;
	FMOD::System *				mpSystem;
	FMOD::MusicSystem *			mpMusicSystem;
	FMOD::EventCategory *		mpMasterCategory;
	FMOD::EventCategory *		mpMusicCategory;
	VuJsonContainer				mInfo;
	DuckingCategories			mDuckingCategories;
	VUUINT32					mNextDuckingID;
	ReverbPresets				mReverbPresets;

	typedef std::map<std::string, int> CategoryPauseCount;
	CategoryPauseCount			mCategoryPauseCount;
	int							mMasterPauseCount;
};

#endif // VU_DISABLE_AUDIO


#define FMODCALL(_call) \
{ \
	FMOD_RESULT _result = (_call); \
	if (_result != FMOD_OK) \
	{ \
		VUWARNING("FMOD error! (%d) %s (%s, %d)", _result, FMOD_ErrorString(_result), __FILE__, __LINE__); \
	} \
}


#include "VuAudio.inl"
