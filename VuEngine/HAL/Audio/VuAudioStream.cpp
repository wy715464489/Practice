//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Wrapper for an FMOD audio stream.
// 
//*****************************************************************************

#include "VuAudioStream.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAudioStreamAsset.h"
#include "VuEngine/HAL/Audio/VuAudio.h"


#if !VU_DISABLE_AUDIO

//*****************************************************************************
VuAudioStream::VuAudioStream():
	mpAsset(VUNULL),
	mpStream(VUNULL),
	mpSound(VUNULL),
	mpChannel(VUNULL)
{
}

//*****************************************************************************
VuAudioStream::~VuAudioStream()
{
	release();
}

//*****************************************************************************
bool VuAudioStream::create(const char *assetName, bool looping)
{
	release();

	if ( !VuAssetFactory::IF()->doesAssetExist<VuAudioStreamAsset>(assetName) )
		return false;

	mpAsset = VuAssetFactory::IF()->createAsset<VuAudioStreamAsset>(assetName);

	FMOD_CREATESOUNDEXINFO info;
	memset(&info, 0, sizeof(info));
	info.cbsize = sizeof(info);
	info.length = mpAsset->data().size();

	FMOD_MODE mode = FMOD_OPENMEMORY;
	if ( looping )
		mode |= FMOD_LOOP_NORMAL;

	FMODCALL(VuAudio::IF()->system()->createStream((const char *)&mpAsset->data().begin(), mode, &info, &mpStream));
	if ( mpStream )
	{
		int numSubSounds = 0;
		FMODCALL(mpStream->getNumSubSounds(&numSubSounds));
		if ( numSubSounds )
		{
			FMODCALL(mpStream->getSubSound(0, &mpSound));
		}
		else
		{
			mpSound = mpStream;
		}
	}

	return true;
}

//*****************************************************************************
void VuAudioStream::release()
{
	stop();

	mpSound = VUNULL;

	if ( mpStream )
	{
		FMODCALL(mpStream->release());
		mpStream = VUNULL;
	}

	if ( mpAsset )
	{
		VuAssetFactory::IF()->releaseAsset(mpAsset);
		mpAsset = VUNULL;
	}
}

//*****************************************************************************
void VuAudioStream::play(bool paused, const char *category)
{
	FMODCALL(VuAudio::IF()->system()->playSound(FMOD_CHANNEL_FREE, mpSound, paused, &mpChannel));

	if ( category )
	{
		FMOD::EventCategory *pCategory;
		if ( VuAudio::IF()->eventSystem()->getCategory(category, &pCategory) == FMOD_OK )
		{
			FMOD::ChannelGroup *pChannelGroup;
			if ( pCategory->getChannelGroup(&pChannelGroup) == FMOD_OK )
			{
				mpChannel->setChannelGroup(pChannelGroup);
			}
		}
	}
}

//*****************************************************************************
void VuAudioStream::stop()
{
	if ( mpChannel )
	{
		mpChannel->stop();
		mpChannel = VUNULL;
	}
}

//*****************************************************************************
void VuAudioStream::setPaused(bool paused)
{
	if ( mpChannel )
		mpChannel->setPaused(paused);
}

//*****************************************************************************
bool VuAudioStream::isPlaying()
{
	bool playing = false;

	if ( mpChannel )
		mpChannel->isPlaying(&playing);

	return playing;
}

//*****************************************************************************
void VuAudioStream::setVolume(float volume)
{
	if ( mpChannel )
		mpChannel->setVolume(volume);
}

//*****************************************************************************
void VuAudioStream::setSpeakerMix(float frontleft, float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright)
{
	if ( mpChannel )
		mpChannel->setSpeakerMix(frontleft, frontright, center, lfe, backleft, backright, sideleft, sideright);
}


#endif // !VU_DISABLE_AUDIO
