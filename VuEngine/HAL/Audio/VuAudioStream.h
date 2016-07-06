//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Wrapper for an FMOD audio stream.
// 
//*****************************************************************************

#pragma once

class VuAudioStreamAsset;
namespace FMOD { class Sound; class Channel; }


#if VU_DISABLE_AUDIO

class VuAudioStream
{
public:
	bool			create(const char *assetName, bool looping) { return false; }
	void			release() {}

	void			play(bool paused, const char *category = VUNULL) {}
	void			stop() {}

	void			setPaused(bool paused) {}
	bool			isPlaying() { return false; }

	void			setVolume(float volume) {}
	void			setSpeakerMix(float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright) {}
};

#else // VU_DISABLE_AUDIO

class VuAudioStream
{
public:
	VuAudioStream();
	~VuAudioStream();

	bool			create(const char *assetName, bool looping);
	void			release();

	void			play(bool paused, const char *category = VUNULL);
	void			stop();

	void			setPaused(bool paused);
	bool			isPlaying();

	void			setVolume(float volume);
	void			setSpeakerMix(float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright);

private:
	VuAudioStreamAsset	*mpAsset;
	FMOD::Sound			*mpStream;
	FMOD::Sound			*mpSound;
	FMOD::Channel		*mpChannel;
};

#endif // VU_DISABLE_AUDIO