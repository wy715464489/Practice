//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 ProfileManager class
// 
//*****************************************************************************

#include <save_data_dialog.h>
#include <message_dialog.h>

#include "VuEngine/Managers/Ps4/VuPs4ProfileManager.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Events/VuEventManager.h"
#include "VuEngine/HAL/Thread/VuThread.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/DB/VuStringDB.h"
#include "VuEngine/Json/VuJsonBinaryReader.h"
#include "VuEngine/Json/VuJsonBinaryWriter.h"
#include "VuEngine/Util/VuHash.h"

#define SAVE_DATA_MEMORY_SIZE		(256*1024)

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuProfileManager, VuPs4ProfileManager);

//*****************************************************************************
bool VuPs4ProfileManager::init(const std::string &gameName)
{
	VuTickManager::IF()->registerHandler(this, &VuPs4ProfileManager::tickInput, "Input");

	// Query system for user id of current user before initializing their
	// profile
	mCurUser = 0;
	mhSaveThread = VUNULL;
	mCachedSaveData.clear();
	mSecondaryCachedSaveData.clear();

	VUINT32 result = sceUserServiceGetInitialUser(&mCurUser);
	if (result != SCE_OK)
	{
		return false;
	}

	char name[32];
	result = sceUserServiceGetUserName(mCurUser, name, sizeof(name) - 1);

	if (result != SCE_OK)
	{
		VUPRINTF("ERROR: sceUserServiceGetUserName() returned Error=%#.8x\n");
	}
	
	VUPRINTF("VuPs4ProfileManager::init(): Initial user is %s.\n", name);

	SceSaveDataMemoryParam param;
	memset(&param, 0x00, sizeof(param));
	strncpy(param.title, "Beach Buggy Racing", sizeof(param.title) - 1);
	param.title[sizeof(param.title) - 1] = '\0';

	result = sceSaveDataSetupSaveDataMemory(mCurUser, SAVE_DATA_MEMORY_SIZE, &param);
	if (result < SCE_OK)
	{
		VUPRINTF("ERROR: sceSaveDataSetupSaveDataMemory : 0x%08x\n", result);
		return false;
	}

	// Now let the profile manager init normally
	if (!VuProfileManager::init(gameName))
	{
		return false;
	}

	return true;
}

//*****************************************************************************

void VuPs4ProfileManager::tickInput(float fdt)
{
	// If there's no thread running, and we have a secondary cache, 
	// create a thread to write that data
	if (mhSaveThread == VUNULL 
		&&
		!mSecondaryCachedSaveData.isNull())
	{
		mCachedSaveData = mSecondaryCachedSaveData;

		mSecondaryCachedSaveData.clear();

		mhSaveThread = VuThread::IF()->createThread(saveThreadProc, this);
	}
}

//*****************************************************************************
void VuPs4ProfileManager::loadInternal()
{
	SceUserServiceUserId curUser = VuPs4ProfileManager::IF()->getCurUser();

	VUUINT8 *pLoadBuffer = new VUUINT8[SAVE_DATA_MEMORY_SIZE];
	if (!pLoadBuffer)
	{
		VUPRINTF("ERROR: out of memory : pLoadBuffer\n");
		return;
	}

	eResult result = RESULT_CORRUPTED;

	VUINT32 retval = sceSaveDataGetSaveDataMemory(curUser, pLoadBuffer, SAVE_DATA_MEMORY_SIZE, 0);
	if (retval < SCE_OK)
	{
		VUPRINTF("sceSaveDataGetSaveDataMemory : 0x%08x\n", retval);
		delete[] pLoadBuffer;
		return;
	}

	// Load the Header
	ProfileHeader header;
	int headerSize = sizeof(header);

	memcpy_s(&header, sizeof(header), pLoadBuffer, sizeof(header));

	// read header
	if (header.mMagic == scProfileMagic && header.mVersion == PROFILE_VERSION)
	{
		// allocate memory
		int dataSize = header.mDataSize;

		char *pData = new char[dataSize];

		memcpy_s(pData, dataSize, pLoadBuffer + headerSize, dataSize);

		// verify hash
		VUUINT32 hash = VuHash::fnv32(pData, dataSize);
		if (header.mDataHash == hash)
		{
			// read json data from memory
			VuJsonBinaryReader reader;

			if (reader.loadFromMemory(mData, pData, dataSize))
			{
				result = RESULT_OK;
			}
		}

		// clean up
		delete[] pData;
	}

	delete[] pLoadBuffer;

	if (result != RESULT_OK)
	{
		mData.clear();
	}
}

//*****************************************************************************
void VuPs4ProfileManager::saveInternal()
{
	if (mhSaveThread)
	{
		mSecondaryCachedSaveData = mData;

		return;
	}

	mCachedSaveData = mData;

	mhSaveThread = VuThread::IF()->createThread(saveThreadProc, this);
}

//*****************************************************************************
void VuPs4ProfileManager::saveThreadProc()
{
	SceUserServiceUserId curUser = VuPs4ProfileManager::IF()->getCurUser();

	VUUINT8 *pSaveBuffer = new VUUINT8[SAVE_DATA_MEMORY_SIZE];
	if (!pSaveBuffer)
	{
		VUPRINTF("ERROR: out of memory : pSaveBuffer\n");
		return;
	}

	ProfileHeader header;

	// calculate sizes
	int headerSize = sizeof(header);
	int dataSize = VuJsonBinaryWriter::calculateDataSize(mCachedSaveData);

	// allocate memory
	char *pData = new char[dataSize];

	// write data to memory
	VuJsonBinaryWriter writer;
	if (writer.saveToMemory(mCachedSaveData, pData, dataSize))
	{
		// fill in header
		header.mMagic = scProfileMagic;
		header.mVersion = PROFILE_VERSION;
		header.mDataSize = dataSize;
		header.mDataHash = VuHash::fnv32(pData, dataSize);

		memcpy_s(pSaveBuffer, SAVE_DATA_MEMORY_SIZE, &header, headerSize);
		memcpy_s(pSaveBuffer + headerSize, SAVE_DATA_MEMORY_SIZE - headerSize, pData, dataSize);

		VUINT32 result = sceSaveDataSetSaveDataMemory(curUser, pSaveBuffer, SAVE_DATA_MEMORY_SIZE, 0);
		if (result < SCE_OK)
		{
			VUPRINTF("ERROR: sceSaveDataSetSaveDataMemory : 0x%08x\n", result);
		}
	}

	// clean up
	delete[] pData;
	delete[] pSaveBuffer;;

	mCachedSaveData.clear();

	mhSaveThread = VUNULL;

	VuThread::IF()->endThread();
}