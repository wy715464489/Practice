//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Audio Project Asset class
// 
//*****************************************************************************

#include "VuAudioProjectAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/VuEngine.h"
#include "VuEngine/HAL/Audio/VuAudio.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuFileUtil.h"


IMPLEMENT_RTTI(VuAudioProjectAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuAudioProjectAsset);


//*****************************************************************************
VuAudioProjectAsset::VuAudioProjectAsset():
	mpProject(VUNULL)
{
}

//*****************************************************************************
void VuAudioProjectAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Audio");

	VuAssetUtil::addFileProperty(schema, "File", "fev");
}

//*****************************************************************************
bool VuAudioProjectAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();

	VuArray<VUBYTE> data;
	if ( !VuFileUtil::loadFile(VuFile::IF()->getRootPath() + fileName, data) )
		return VUWARNING("Unable to load generic asset %s.", fileName.c_str());

	writer.writeValue(data.size());
	writer.writeData(&data.begin(), data.size());

	return true;
}

//*****************************************************************************
bool VuAudioProjectAsset::load(VuBinaryDataReader &reader)
{
	int dataSize;
	reader.readValue(dataSize);
	const void *pData = reader.cur();
	reader.skip(dataSize);

#if !VU_DISABLE_AUDIO

	FMOD_EVENT_LOADINFO loadInfo;
	memset(&loadInfo, 0, sizeof(loadInfo));
	loadInfo.size = sizeof(loadInfo);
	loadInfo.loadfrommemory_length = dataSize;
	FMODCALL(VuAudio::IF()->eventSystem()->load((const char *)pData, &loadInfo, &mpProject));

	if ( VuEngine::IF()->gameMode() )
	{
		// preload sample data
		if ( mpProject )
		{
			int eventCount = 0;
			FMODCALL(mpProject->getNumEvents(&eventCount));
			if ( eventCount )
			{
				int *eventArray = new int[eventCount];
				for ( int i = 0; i < eventCount; i++ )
					eventArray[i] = i;

				FMODCALL(mpProject->loadSampleData(eventArray, eventCount, VUNULL, 0, FMOD_EVENT_DEFAULT));

				// clean up
				delete[] eventArray;
			}
		}

		VuAudio::IF()->enumerateRevertPresets();
	}

#endif // !VU_DISABLE_AUDIO

	return true;
}

//*****************************************************************************
void VuAudioProjectAsset::unload()
{
#if !VU_DISABLE_AUDIO
	if ( mpProject )
	{
		FMODCALL(mpProject->release());
		mpProject = VUNULL;
	}
#endif // !VU_DISABLE_AUDIO
}