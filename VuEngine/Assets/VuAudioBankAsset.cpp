//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Audio Bank Asset class
// 
//*****************************************************************************

#include "VuAudioBankAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/VuEngine.h"
#include "VuEngine/HAL/Audio/VuAudio.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Util/VuFileUtil.h"


IMPLEMENT_RTTI(VuAudioBankAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuAudioBankAsset);


//*****************************************************************************
VuAudioBankAsset::VuAudioBankAsset():
	mInstanceCount(1),
	mDecompress(true),
	mpSound(VUNULL)
{
}

//*****************************************************************************
void VuAudioBankAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("Audio");

	VuAssetUtil::addFileProperty(schema, "File", "fsb");
	VuAssetUtil::addIntProperty(schema, "InstanceCount", 1);
	VuAssetUtil::addBoolProperty(schema, "Decompress", true);
}

//*****************************************************************************
bool VuAudioBankAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();

	VuArray<VUBYTE> data;
	if ( !VuFileUtil::loadFile(VuFile::IF()->getRootPath() + fileName, data) )
		return VUWARNING("Unable to load generic asset %s.", fileName.c_str());

	writer.writeValue(data.size());
	writer.writeData(&data.begin(), data.size());

	// instance count
	int instanceCount = 1;
	VuDataUtil::getValue(creationInfo["InstanceCount"], instanceCount);
	writer.writeValue(instanceCount);

	// decompress
	bool decompress = true;
	VuDataUtil::getValue(creationInfo["Decompress"], decompress);
	writer.writeValue(decompress);

	// bank name
	std::string bankName = VuFileUtil::getNameExt(fileName);
	writer.writeString(bankName.c_str());

	return true;
}

//*****************************************************************************
bool VuAudioBankAsset::load(VuBinaryDataReader &reader)
{
	int dataSize;
	reader.readValue(dataSize);
	const void *pData = reader.cur();
	reader.skip(dataSize);
	reader.readValue(mInstanceCount);
	reader.readValue(mDecompress);
	reader.readString(mBankName);

#if !VU_DISABLE_AUDIO

	if ( VuEngine::IF()->gameMode() )
	{
		// load bank
		FMOD_MODE mode = FMOD_OPENMEMORY;
		if ( !mDecompress )
			mode |= FMOD_CREATECOMPRESSEDSAMPLE;
		FMOD_CREATESOUNDEXINFO createInfo;
		memset(&createInfo, 0, sizeof(createInfo));
		createInfo.cbsize = sizeof(createInfo);
		createInfo.length = dataSize;
		createInfo.encryptionkey = VuAudio::IF()->getEncryptionKey();
		FMODCALL(VuAudio::IF()->system()->createSound((const char *)pData, mode, &createInfo, &mpSound));

		// preload
		for ( int i = 0; i < mInstanceCount; i++ )
			FMODCALL(VuAudio::IF()->eventSystem()->preloadFSB(mBankName.c_str(), i, mpSound));
	}

#endif // !VU_DISABLE_AUDIO

	return true;
}

//*****************************************************************************
void VuAudioBankAsset::unload()
{
#if !VU_DISABLE_AUDIO

	if ( VuEngine::IF()->gameMode() )
	{
		for ( int i = 0; i < mInstanceCount; i++ )
			FMODCALL(VuAudio::IF()->eventSystem()->unloadFSB(mBankName.c_str(), i));

		if ( mpSound )
		{
			FMODCALL(mpSound->release());
			mpSound = VUNULL;
		}
	}

#endif // !VU_DISABLE_AUDIO
}