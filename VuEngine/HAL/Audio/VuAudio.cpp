//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  interface class for Audio.
// 
//*****************************************************************************

#include "VuEngine/VuEngine.h"
#include "VuEngine/HAL/Audio/VuAudio.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAssetBakery.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/HAL/Gfx/VuGfxTypes.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Math/VuUnitConversion.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/Util/VuColor.h"
#include "VuEngine/Dev/VuDev.h"
#include "VuEngine/Dev/VuDevMenu.h"
#include "VuEngine/Dev/VuDevStat.h"

#if defined VUANDROID
	#include "VuEngine/HAL/Sys/Android/VuAndroidSys.h"
#endif

// the interface
#if !defined VUANDROID
	IMPLEMENT_SYSTEM_COMPONENT(VuAudio, VuAudio);
#endif

#if !VU_DISABLE_AUDIO

// constants
#define ENABLE_PROFILE 0
#define MAX_PLAYING_EVENTS 256
#define MAX_WAVE_BANKS 32

// static variables
static bool sbDebugDrawEmitters = false;


//*****************************************************************************
VuAudio::VuAudio():
	mstrEncryptionKey(VUNULL),
	mpEventSystem(VUNULL),
	mpSystem(VUNULL),
	mpMusicSystem(VUNULL),
	mNextDuckingID(1),
	mMasterPauseCount(0)
{
}

//*****************************************************************************
bool VuAudio::init()
{
	mstrEncryptionKey = VuEngine::IF()->options().mstrAudioEncryptionKey;

	FMODCALL(EventSystem_Create(&mpEventSystem));
	FMODCALL(mpEventSystem->getSystemObject(&mpSystem));
	FMODCALL(mpEventSystem->getMusicSystem(&mpMusicSystem));

	VUUINT version;
	FMODCALL(mpSystem->getVersion(&version));
	if ( version < FMOD_VERSION )
	{
		VUPRINTF("Error! You are using an old version of FMOD %08x. This program requires %08x\n", version, FMOD_VERSION);
		return false;
	}

	void *pExtraDriverData = VUNULL;

	// disable file system
	FMODCALL(mpSystem->setFileSystem(VUNULL, VUNULL, VUNULL, VUNULL, VUNULL, VUNULL, -1));

	// platform-specific initialization
#if defined VUANDROID

	if ( VuEngine::IF()->options().mUseAudioTrack )
	{
		FMODCALL(mpSystem->setOutput(FMOD_OUTPUTTYPE_AUDIOTRACK));
	}

	if ( VuEngine::IF()->options().mSurroundSound && VuEngine::IF()->options().mUseAudioTrack && VuAndroidSys::IF()->getOsBuildVersion() >= 16 )
	{
		FMODCALL(mpSystem->setSpeakerMode(FMOD_SPEAKERMODE_5POINT1));
	}
	else
	{
		FMODCALL(mpSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO));
	}

#elif defined VUIOS

	FMOD_IPHONE_EXTRADRIVERDATA extraDriverData;
	memset(&extraDriverData, 0, sizeof(extraDriverData));
	extraDriverData.sessionCategory = FMOD_IPHONE_SESSIONCATEGORY_AMBIENTSOUND;
	pExtraDriverData = &extraDriverData;

	FMODCALL(mpSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO));

#else

	int numDrivers;
	FMODCALL(mpSystem->getNumDrivers(&numDrivers));
	if ( numDrivers == 0 )
	{
		FMODCALL(mpSystem->setOutput(FMOD_OUTPUTTYPE_NOSOUND));
	}
	else
	{
		FMOD_SPEAKERMODE speakerMode;
		FMODCALL(mpSystem->getDriverCaps(0, VUNULL, VUNULL, &speakerMode));
		FMODCALL(mpSystem->setSpeakerMode(speakerMode));
	}

#endif


	FMOD_INITFLAGS flags = FMOD_INIT_3D_RIGHTHANDED | FMOD_INIT_VOL0_BECOMES_VIRTUAL;
#if ENABLE_PROFILE
	flags |= FMOD_INIT_ENABLE_PROFILE;
#endif

	FMODCALL(mpEventSystem->init(VuEngine::IF()->options().mAudioMaxChannels, flags, pExtraDriverData));

	// get categories
	FMODCALL(mpEventSystem->getCategory("master", &mpMasterCategory));
	FMODCALL(mpEventSystem->getCategory("music", &mpMusicCategory));

	// register phased tick
	VuTickManager::IF()->registerHandler(this, &VuAudio::tickAudio, "Audio");

	return true;
}

//*****************************************************************************
void VuAudio::release()
{
	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);

	FMODCALL(mpEventSystem->release());
}

//*****************************************************************************
void VuAudio::postInit()
{
	if ( VuDevStat::IF() )
		VuDevStat::IF()->addPage("Audio", VuRect(10, 10, 80, 80));

	if ( VuDevMenu::IF() )
		VuDevMenu::IF()->addBool("Audio/Draw Emitters", sbDebugDrawEmitters);

	if ( VuEngine::IF()->editorMode() )
		loadInfo();
}

//*****************************************************************************
bool VuAudio::getMinMaxDist(FMOD::Event *pEvent, float &minDist, float &maxDist)
{
	// get mode
	FMOD_MODE mode;
	pEvent->getPropertyByIndex(FMOD_EVENTPROPERTY_MODE, &mode, true);

	// get distance parameter
	FMOD::EventParameter *pDistanceParam;
	pEvent->getParameter("(distance)", &pDistanceParam);

	if ( mode != FMOD_3D && !pDistanceParam )
		return false;

	// get 3d rolloff property
	FMOD_MODE rolloffMode;
	pEvent->getPropertyByIndex(FMOD_EVENTPROPERTY_3D_ROLLOFF, &rolloffMode, true);

	// determine min/max dist
	if ( (mode == FMOD_2D || rolloffMode == FMOD_3D_CUSTOMROLLOFF) && pDistanceParam )
	{
		pDistanceParam->getRange(&minDist, &maxDist);
	}
	else
	{
		pEvent->getPropertyByIndex(FMOD_EVENTPROPERTY_3D_MINDISTANCE, &minDist, true);
		pEvent->getPropertyByIndex(FMOD_EVENTPROPERTY_3D_MAXDISTANCE, &maxDist, true);
	}

	return true;
}

//*****************************************************************************
void VuAudio::setListenerCount(int count)
{
	mpEventSystem->set3DNumListeners(count);
}

//*****************************************************************************
void VuAudio::setListenerAttributes(int index, const VuVector3 &pos, const VuVector3 &vel, const VuVector3 &fwd, const VuVector3 &up)
{
	mpEventSystem->set3DListenerAttributes(index, reinterpret_cast<const FMOD_VECTOR *>(&pos), reinterpret_cast<const FMOD_VECTOR *>(&vel), reinterpret_cast<const FMOD_VECTOR *>(&fwd), reinterpret_cast<const FMOD_VECTOR *>(&up));
}

//*****************************************************************************
VUUINT32 VuAudio::startDucking(const char *strCategory, float volume, float maxDuration)
{
	if ( volume < 0.0f )
	{
		FMOD::EventCategory *pCategory;
		if ( VuAudio::IF()->eventSystem()->getCategory(strCategory, &pCategory) == FMOD_OK )
		{
			VuDuckingEntry entry;
			entry.mID = mNextDuckingID++;
			entry.mAge = 0.0f;
			entry.mDuration = maxDuration;
			entry.mVolume = volume;

			mDuckingCategories[pCategory].push_back(entry);

			return entry.mID;
		}
	}

	return 0;
}

//*****************************************************************************
void VuAudio::stopDucking(VUUINT32 id)
{
	for ( DuckingCategories::iterator iter = mDuckingCategories.begin(); iter != mDuckingCategories.end(); iter++ )
	{
		DuckingEntries &entries = iter->second;
		for ( int i = 0; i < entries.size(); i++ )
		{
			if ( entries[i].mID == id )
			{
				entries[i].mDuration = VuMin(entries[i].mDuration, entries[i].mAge);
				return;
			}
		}
	}
}

//*****************************************************************************
void VuAudio::enumerateRevertPresets()
{
	int count;
	if ( VuAudio::IF()->eventSystem()->getNumReverbPresets(&count) == FMOD_OK )
	{
		for ( int index = 0; index < count; index++ )
		{
			FMOD_REVERB_PROPERTIES props;
			char *strName;
			if ( VuAudio::IF()->eventSystem()->getReverbPresetByIndex(index, &props, &strName) == FMOD_OK )
			{
				mReverbPresets[strName] = props;
			}
		}
	}
}

//*****************************************************************************
bool VuAudio::getReverbPreset(const std::string &strName, FMOD_REVERB_PROPERTIES &props)
{
	ReverbPresets::const_iterator iter = mReverbPresets.find(strName);
	if ( iter != mReverbPresets.end() )
	{
		props = iter->second;
		return true;
	}

	return false;
}

//*****************************************************************************
bool VuAudio::isOtherAudioPlaying()
{
#ifdef VUIOS
	bool playing = false;
	if ( FMOD_IPhone_OtherAudioIsPlaying(&playing) == FMOD_OK )
		return playing;
#endif

	return false;
}

//*****************************************************************************
void VuAudio::pushCategoryPause(const char *category)
{
	CategoryPauseCount::iterator iter = mCategoryPauseCount.find(category);
	if ( iter == mCategoryPauseCount.end() )
		mCategoryPauseCount[category] = 1;
	else
		iter->second++;

	FMOD::EventCategory *pCategory;
	if ( VuAudio::IF()->eventSystem()->getCategory(category, &pCategory) == FMOD_OK )
		pCategory->setPaused(true);
}

//*****************************************************************************
void VuAudio::popCategoryPause(const char *category)
{
	CategoryPauseCount::iterator iter = mCategoryPauseCount.find(category);
	VUASSERT(iter != mCategoryPauseCount.end() && iter->second > 0, "Audio Master Pause push/pop mismatch!");
	if ( iter != mCategoryPauseCount.end() )
	{
		iter->second--;

		if ( iter->second == 0 )
		{
			FMOD::EventCategory *pCategory;
			if ( VuAudio::IF()->eventSystem()->getCategory(category, &pCategory) == FMOD_OK )
				pCategory->setPaused(false);
		}
	}
}

//*****************************************************************************
void VuAudio::pushMasterPause()
{
	mMasterPauseCount++;

	mpMasterCategory->setPaused(true);
}

//*****************************************************************************
void VuAudio::popMasterPause()
{
	VUASSERT(mMasterPauseCount > 0, "Audio Master Pause push/pop mismatch!");

	mMasterPauseCount--;

	if ( mMasterPauseCount == 0 )
		mpMasterCategory->setPaused(false);
}

//*****************************************************************************
void VuAudio::tickAudio(float fdt)
{
	// update ducking
	updateDucking(fdt);

	FMODCALL(mpEventSystem->update());

	// dev stats
	updateDevStats();

	// debug draw instigators
	if ( sbDebugDrawEmitters )
		drawEmitters();
}

//*****************************************************************************
void VuAudio::updateDucking(float fdt)
{
	for ( DuckingCategories::iterator iter = mDuckingCategories.begin(); iter != mDuckingCategories.end(); iter++ )
	{
		DuckingEntries &entries = iter->second;
		if ( entries.size() )
		{
			float fTotalVolume = 0.0f;
			for ( int i = 0; i < entries.size(); i++ )
			{
				entries[i].mAge += fdt;
				float fEntryVolume = entries[i].calcVolume();
				if ( fEntryVolume >= 0.0f )
				{
					entries.swap(i, entries.size() - 1);
					entries.pop_back();
					i--;
				}
				fTotalVolume = VuMin(fTotalVolume, fEntryVolume);
			}

			float fLinearVolume = VuDecibelsToRatio(fTotalVolume);
			iter->first->setVolume(fLinearVolume);
		}
	}
}

//*****************************************************************************
void VuAudio::updateDevStats()
{
	if ( !VuDevStat::IF() )
		return;

	VuDevStatPage *pPage = VuDevStat::IF()->getCurPage();
	if ( pPage == VUNULL )
		return;

	if ( pPage->getName() != "Audio" )
		return;

	pPage->clear();

	// get info
	FMOD::Event *playingEvents[MAX_PLAYING_EVENTS];
	FMOD_EVENT_WAVEBANKINFO waveBankInfo[MAX_WAVE_BANKS];

	FMOD_EVENT_SYSTEMINFO info;
	memset(&info, 0, sizeof(info));
	info.numplayingevents = MAX_PLAYING_EVENTS;
	info.playingevents = (FMOD_EVENT **)playingEvents;
	info.maxwavebanks = MAX_WAVE_BANKS;
	info.wavebankinfo = waveBankInfo;
	FMODCALL(mpEventSystem->getInfo(&info));

	// cpu usage
	{
		float dsp, stream, geometry, update, total;
		FMODCALL(mpSystem->getCPUUsage(&dsp, &stream, &geometry, &update, &total));
		pPage->printf("dsp cpu:      %4.1f%%\n", dsp);
		pPage->printf("stream cpu:   %4.1f%%\n", stream);
		pPage->printf("geometry cpu: %4.1f%%\n", geometry);
		pPage->printf("update cpu:   %4.1f%%\n", update);
		pPage->printf("total cpu:    %4.1f%%\n", total);
	}

	// memory usage
	{
		int curAlloc, maxAlloc;
		FMODCALL(FMOD::Memory_GetStats(&curAlloc, &maxAlloc, false));
		pPage->printf("cur mem: %dK\n", curAlloc/1024);
		pPage->printf("max mem: %dK\n", maxAlloc/1024);
	}

	// wavebanks
	{
		pPage->printf("            Wavebank StrRefCnt SamRefCnt NumStr MaxStr StrInUse StrMemory SamMemory\n");
		for ( int i = 0; i < info.maxwavebanks; i++ )
		{
			pPage->printf("%20s ", waveBankInfo[i].name);
			pPage->printf("%9d ", waveBankInfo[i].streamrefcnt);
			pPage->printf("%9d ", waveBankInfo[i].samplerefcnt);
			pPage->printf("%6d ", waveBankInfo[i].numstreams);
			pPage->printf("%6d ", waveBankInfo[i].maxstreams);
			pPage->printf("%8d ", waveBankInfo[i].streamsinuse);
			pPage->printf("%9d ", waveBankInfo[i].streammemory);
			pPage->printf("%9d ", waveBankInfo[i].samplememory);
			pPage->printf("\n");
		}
		pPage->printf("\n");
	}

	// events
	int channelsUsed = 0;
	for ( int i = 0; i < VuMin(info.numplayingevents, MAX_PLAYING_EVENTS); i++ )
	{
		FMOD_EVENT_INFO info;
		playingEvents[i]->getInfo(VUNULL, VUNULL, &info);
		channelsUsed += info.channelsplaying;
	}

	pPage->printf("NumEvents: %d\n", info.numevents);
	pPage->printf("NumInstances: %d\n", info.numinstances);
	pPage->printf("NumPlayingEvents: %d\n", info.numplayingevents);
	pPage->printf("NumChannelsUsed: %d\n", channelsUsed);
	pPage->printf("{");
	for ( int i = 0; i < VuMin(info.numplayingevents, MAX_PLAYING_EVENTS); i++ )
	{
		char *name;
		playingEvents[i]->getInfo(VUNULL, &name, VUNULL);
		pPage->printf(" %s", name);
	}
	pPage->printf(" }\n");
}

//*****************************************************************************
void VuAudio::drawEmitters()
{
	FMOD::Event *playingEvents[MAX_PLAYING_EVENTS];

	FMOD_EVENT_SYSTEMINFO info;
	memset(&info, 0, sizeof(info));
	info.numplayingevents = MAX_PLAYING_EVENTS;
	info.playingevents = (FMOD_EVENT **)playingEvents;
	FMODCALL(mpEventSystem->getInfo(&info));

	for ( int i = 0; i < VuMin(info.numplayingevents, MAX_PLAYING_EVENTS); i++ )
	{
		FMOD::Event *pEvent = playingEvents[i];

		// determine min/max dist
		float minDist = 0, maxDist = 0;
		if ( getMinMaxDist(pEvent, minDist, maxDist) )
		{
			// get position
			FMOD_VECTOR fmodPos;
			pEvent->get3DAttributes(&fmodPos, VUNULL);
			VuVector3 vPos = VuAudio::toVuVector3(fmodPos);

			// draw sphere(s)
			if ( minDist > 0 )
				VuDev::IF()->drawSphere(vPos, minDist, VuColor(192,64,64), 8, 8);

			if ( maxDist > minDist )
				VuDev::IF()->drawSphere(vPos, maxDist, VuColor(64,192,64), 8, 8);

			// draw name
			char *name;
			pEvent->getInfo(VUNULL, &name, VUNULL);
			VuDev::IF()->printf(vPos, VUGFX_TEXT_DRAW_HCENTER|VUGFX_TEXT_DRAW_VCENTER, VuColor(192, 192, 64), name);
		}

		// get mode
		FMOD_MODE mode;
		pEvent->getPropertyByIndex(FMOD_EVENTPROPERTY_MODE, &mode, true);

		// get distance parameter
		FMOD::EventParameter *pDistanceParam;
		pEvent->getParameter("(distance)", &pDistanceParam);
	}
}

//*****************************************************************************
void VuAudio::loadInfo()
{
	VuAssetFactory::AssetNames projectNames = VuAssetFactory::IF()->getAssetNames("VuAudioProjectAsset");
	for ( int iProject = 0; iProject < (int)projectNames.size(); iProject++ )
	{
		const std::string &projectName = projectNames[iProject];
		const VuJsonContainer &assetInfo = VuAssetBakery::IF()->getCreationInfo(VUPLATFORM, VuAssetFactory::IF()->getSku(), VuSys::IF()->getLanguage(), "VuAudioProjectAsset", projectName);
		std::string fileName = assetInfo["File"].asString();
		if ( fileName.length() )
		{
			fileName = VuFileUtil::getPathName(fileName) + ".txt";

			// load file
			VuArray<VUBYTE> data(0);
			if ( VuFileUtil::loadFile(VuFile::IF()->getRootPath() + fileName, data) )
			{
				data.resize(data.size() + 1);
				data.back() = '\0';

				char *strFile = (char *)&data.begin();

				// break string up into smaller chunks
				char *strEvents = strstr(strFile, "# Events");
				char *strGroups = strstr(strFile, "# Groups");
				char *strCategories = strstr(strFile, "# Categories");
				char *strReverbs = strstr(strFile, "# Reverbs");
				char *strMusicCues = strstr(strFile, "# Music Cues");

				strEvents[-1] = '\0';
				strGroups[-1] = '\0';
				strCategories[-1] = '\0';
				strReverbs[-1] = '\0';
				strMusicCues[-1] = '\0';

				// parse info
				parseInfo(projectName, strEvents, mInfo["Events"]);
				parseInfo(projectName, strGroups, mInfo["Groups"]);
				parseInfo(projectName, strCategories, mInfo["Categories"]);
				parseInfo("", strReverbs, mInfo["Reverbs"]);
			}
			else
			{
				VUPRINTF("Error loading file: '%s'\n", fileName.c_str());
			}
		}
	}
}

//*****************************************************************************
void VuAudio::parseInfo(const std::string &projectName, const char *strText, VuJsonContainer &info)
{
	while ( (strText = strstr(strText, "Name            : ")) != VUNULL)
	{
		char strName[256];

		strText += 18;
		int len = (int)strcspn(strText, "\n\r");
		VU_STRNCPY(strName, sizeof(strName), strText, len);
		strName[len] = '\0';
		strText += len + 1;
		info.append().putValue(projectName + strName);
	}
}

//*****************************************************************************
float VuAudio::VuDuckingEntry::calcVolume()
{
	if ( mAge > mDuration )
		return 0.0f;

	return mVolume;
}

#endif // VU_DISABLE_AUDIO
