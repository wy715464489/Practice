//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ProfileManager class
// 
//*****************************************************************************

#include "VuProfileManager.h"
#include "VuEngine/VuEngine.h"
#include "VuEngine/Events/VuEventManager.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Json/VuJsonBinaryReader.h"
#include "VuEngine/Json/VuJsonBinaryWriter.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/Util/VuHash.h"

//#define DISABLE_CHECKS_FOR_MICROSOFT_FILE_FUZZING 1


//*****************************************************************************
VuProfileManager::VuProfileManager()
{
}

//*****************************************************************************
bool VuProfileManager::init(const std::string &gameName)
{
	loadInternal();

	return true;
}

//*****************************************************************************
void VuProfileManager::save()
{
	VuEventManager::IF()->broadcast("OnSaveProfile");

	saveInternal();
}

//*****************************************************************************
void VuProfileManager::loadInternal()
{
	std::string path;
	getPath(path);

	if ( loadInternal(path + PROFILE_FILE_NAME, mData) != RESULT_OK )
		loadInternal(path + PROFILE_BACKUP_FILE_NAME, mData);
}

//*****************************************************************************
void VuProfileManager::saveInternal()
{
	std::string path;
	getPath(path);

	if ( saveInternal(path + PROFILE_FILE_NAME, mData) )
		saveInternal(path + PROFILE_BACKUP_FILE_NAME, mData);
}

//*****************************************************************************
VuProfileManager::eResult VuProfileManager::loadInternal(const std::string &fileName, VuJsonContainer &data)
{
	eResult result = RESULT_OK;

	// open file
	VUHANDLE fp = VuFile::IF()->open(fileName, VuFile::MODE_READ);
	if ( fp )
	{
		result = RESULT_CORRUPTED;

		ProfileHeader header;
		int headerSize = sizeof(header);

		// get file size
		int totalSize = VuFile::IF()->size(fp);
		int dataSize = totalSize - headerSize;

		// read header
		if ( VuFile::IF()->read(fp, &header, headerSize) == headerSize )
		{
#if !DISABLE_CHECKS_FOR_MICROSOFT_FILE_FUZZING
			if ( header.mMagic == scProfileMagic && header.mVersion == PROFILE_VERSION && header.mDataSize == (VUUINT32)dataSize )
#endif
			{
				// allocate memory
				char *pData = new char[dataSize];

				// read data from file
				if ( VuFile::IF()->read(fp, pData, dataSize) == dataSize )
				{
					// verify hash
					VUUINT32 hash = VuHash::fnv32(pData, dataSize);
					if ( header.mDataHash == hash )
					{
						// read json data from memory
						VuJsonBinaryReader reader;
						if ( reader.loadFromMemory(data, pData, dataSize) )
							result = RESULT_OK;
					}
				}

				// clean up
				delete[] pData;
			}
		}

		VuFile::IF()->close(fp);
	}
	else
	{
		return RESULT_NOT_FOUND;
	}

	if ( result != RESULT_OK )
		data.clear();

	return result;
}

//*****************************************************************************
bool VuProfileManager::saveInternal(const std::string &fileName, const VuJsonContainer &data)
{
	bool success = false;

	ProfileHeader header;

	// calculate sizes
	int headerSize = sizeof(header);
	int dataSize = VuJsonBinaryWriter::calculateDataSize(data);

	// allocate memory
	char *pData = new char[dataSize];

	// write data to memory
	VuJsonBinaryWriter writer;
	if ( writer.saveToMemory(data, pData, dataSize) )
	{
		// fill in header
		header.mMagic = scProfileMagic;
		header.mVersion = PROFILE_VERSION;
		header.mDataSize = dataSize;
		header.mDataHash = VuHash::fnv32(pData, dataSize);

		// write file
		VUHANDLE fp = VuFile::IF()->open(fileName, VuFile::MODE_WRITE);
		if ( fp )
		{
			if ( VuFile::IF()->write(fp, &header, headerSize) == headerSize )
				if ( VuFile::IF()->write(fp, pData, dataSize) == dataSize )
					success = true;
			VuFile::IF()->close(fp);
		}
	}

	// clean up
	delete[] pData;

	return success;
}
