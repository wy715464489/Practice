//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  AudioDsp entity
// 
//*****************************************************************************

#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Components/Script/VuScriptComponent.h"
#include "VuEngine/Properties/VuBasicProperty.h"
#include "VuEngine/Properties/VuDBEntryProperty.h"
#include "VuEngine/HAL/Audio/VuAudio.h"


#if VU_DISABLE_AUDIO

class VuAudioDspEntity : public VuEntity
{
	DECLARE_RTTI
};
IMPLEMENT_RTTI(VuAudioDspEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAudioDspEntity);

#else // VU_DISABLE_AUDIO

class VuAudioDspEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuAudioDspEntity();

	virtual void		onGameInitialize();
	virtual void		onGameRelease();

private:
	// scripting
	VuRetVal			Activate(const VuParams &params = VuParams());
	VuRetVal			Deactivate(const VuParams &params = VuParams());

	bool				translateType(const char *str, FMOD_DSP_TYPE &type);
	bool				translateParam(const char *str, int &index);

	// components
	VuScriptComponent	*mpScriptComponent;

	// properties
	bool				mbInitiallyActive;
	std::string			mType;
	std::string			mCategory;

	// property references
	VuDBEntryProperty	*mpTypeProperty;

	FMOD::DSP			*mpDsp;
	bool				mbActive;
};


IMPLEMENT_RTTI(VuAudioDspEntity, VuEntity);
IMPLEMENT_ENTITY_REGISTRATION(VuAudioDspEntity);


//*****************************************************************************
VuAudioDspEntity::VuAudioDspEntity():
	mbInitiallyActive(false),
	mpDsp(VUNULL),
	mbActive(false)
{
	// properties
	addProperty(new VuBoolProperty("Initially Active", mbInitiallyActive));
	addProperty(mpTypeProperty = new VuDBEntryProperty("Type", mType, "DspDB"));
	addProperty(new VuStringProperty("Category", mCategory));

	// components
	addComponent(mpScriptComponent = new VuScriptComponent(this, 100));

	// scripting
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAudioDspEntity, Activate);
	ADD_SCRIPT_INPUT_NOARGS(mpScriptComponent, VuAudioDspEntity, Deactivate);
}

//*****************************************************************************
void VuAudioDspEntity::onGameInitialize()
{
	FMOD_DSP_TYPE type;
	if ( translateType(mpTypeProperty->getEntryData()["Type"].asCString(), type) )
	{
		if ( VuAudio::IF()->system()->createDSPByType(type, &mpDsp) == FMOD_OK )
		{
			const VuJsonContainer &params = mpTypeProperty->getEntryData()["Parameters"];
			for ( int iParam = 0; iParam < params.numMembers(); iParam++ )
			{
				const std::string &key = params.getMemberKey(iParam);
				float value = params[key].asFloat();

				int index;
				if ( translateParam(key.c_str(), index) )
				{
					mpDsp->setParameter(index, value);
				}
			}
		}
	}

	if ( mbInitiallyActive )
		Activate();
}

//*****************************************************************************
void VuAudioDspEntity::onGameRelease()
{
	Deactivate();

	if ( mpDsp )
	{
		mpDsp->release();
	}
}

//*****************************************************************************
VuRetVal VuAudioDspEntity::Activate(const VuParams &params)
{
	if ( mpDsp && !mbActive )
	{
		FMOD::EventCategory *pCategory;
		if ( VuAudio::IF()->eventSystem()->getCategory(mCategory.c_str(), &pCategory) == FMOD_OK )
		{
			FMOD::ChannelGroup *pChannelGroup;
			if ( pCategory->getChannelGroup(&pChannelGroup) == FMOD_OK )
			{
				pChannelGroup->addDSP(mpDsp, 0);
			}
		}

		mbActive = true;
	}

	return VuRetVal();
}

//*****************************************************************************
VuRetVal VuAudioDspEntity::Deactivate(const VuParams &params)
{
	if ( mpDsp && mbActive )
	{
		mpDsp->remove();
		mbActive = false;
	}

	return VuRetVal();
}

//*****************************************************************************
bool VuAudioDspEntity::translateType(const char *str, FMOD_DSP_TYPE &type)
{
	#define CHECK_TYPE(name) if ( strcmp(str, #name) == 0 ) { type = FMOD_DSP_TYPE_##name; return true; }

	CHECK_TYPE(LOWPASS);
	CHECK_TYPE(ITLOWPASS);
	CHECK_TYPE(LOWPASS_SIMPLE);
	CHECK_TYPE(HIGHPASS);
	CHECK_TYPE(ECHO);
	CHECK_TYPE(FLANGE);
	CHECK_TYPE(DISTORTION);
	CHECK_TYPE(NORMALIZE);
	CHECK_TYPE(PARAMEQ);
	CHECK_TYPE(PITCHSHIFT);
	CHECK_TYPE(CHORUS);
	CHECK_TYPE(ITECHO);
	CHECK_TYPE(COMPRESSOR);
	CHECK_TYPE(TREMOLO);

	#undef ADD_TYPE

	return false;
}

//*****************************************************************************
bool VuAudioDspEntity::translateParam(const char *str, int &index)
{
	#define CHECK_PARAM(name) if ( strcmp(str, #name) == 0 ) { index = FMOD_DSP_##name; return true; }

	CHECK_PARAM(LOWPASS_CUTOFF);
	CHECK_PARAM(LOWPASS_RESONANCE);
	CHECK_PARAM(ITLOWPASS_CUTOFF);
	CHECK_PARAM(ITLOWPASS_RESONANCE); 
	CHECK_PARAM(LOWPASS_SIMPLE_CUTOFF); 
	CHECK_PARAM(HIGHPASS_CUTOFF);
	CHECK_PARAM(HIGHPASS_RESONANCE);
	CHECK_PARAM(ECHO_DELAY);
	CHECK_PARAM(ECHO_DECAYRATIO);
	CHECK_PARAM(ECHO_MAXCHANNELS);
	CHECK_PARAM(ECHO_DRYMIX);
	CHECK_PARAM(ECHO_WETMIX);
	CHECK_PARAM(FLANGE_DRYMIX);
	CHECK_PARAM(FLANGE_WETMIX);
	CHECK_PARAM(FLANGE_DEPTH);
	CHECK_PARAM(DISTORTION_LEVEL);
	CHECK_PARAM(NORMALIZE_FADETIME);
	CHECK_PARAM(NORMALIZE_THRESHHOLD);
	CHECK_PARAM(NORMALIZE_MAXAMP);
	CHECK_PARAM(PARAMEQ_CENTER);
	CHECK_PARAM(PARAMEQ_BANDWIDTH);
	CHECK_PARAM(PARAMEQ_GAIN);
	CHECK_PARAM(PITCHSHIFT_PITCH);
	CHECK_PARAM(PITCHSHIFT_FFTSIZE);
	CHECK_PARAM(PITCHSHIFT_OVERLAP);
	CHECK_PARAM(PITCHSHIFT_MAXCHANNELS);
	CHECK_PARAM(CHORUS_DRYMIX);
	CHECK_PARAM(CHORUS_WETMIX1);
	CHECK_PARAM(CHORUS_WETMIX2);
	CHECK_PARAM(CHORUS_WETMIX3);
	CHECK_PARAM(CHORUS_DELAY);
	CHECK_PARAM(CHORUS_RATE);
	CHECK_PARAM(CHORUS_DEPTH);
	CHECK_PARAM(ITECHO_WETDRYMIX);
	CHECK_PARAM(ITECHO_FEEDBACK);
	CHECK_PARAM(ITECHO_LEFTDELAY);
	CHECK_PARAM(ITECHO_RIGHTDELAY);
	CHECK_PARAM(ITECHO_PANDELAY);
	CHECK_PARAM(COMPRESSOR_THRESHOLD);
	CHECK_PARAM(COMPRESSOR_ATTACK);
	CHECK_PARAM(COMPRESSOR_RELEASE);
	CHECK_PARAM(COMPRESSOR_GAINMAKEUP);
	CHECK_PARAM(TREMOLO_FREQUENCY);
	CHECK_PARAM(TREMOLO_DEPTH);
	CHECK_PARAM(TREMOLO_SHAPE);
	CHECK_PARAM(TREMOLO_SKEW);
	CHECK_PARAM(TREMOLO_DUTY);
	CHECK_PARAM(TREMOLO_SQUARE);
	CHECK_PARAM(TREMOLO_PHASE);
	CHECK_PARAM(TREMOLO_SPREAD);

	#undef ADD_TYPE

	return false;
}

#endif // VU_DISABLE_AUDIO
