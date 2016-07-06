//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Handles client host IO.
// 
//*****************************************************************************


#include "VuFileHostIO.h"
#include "VuEngine/Math/VuMath.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"
#include "VuEngine/Dev/VuDevHostComm.h"


#define HASH_CACHE_TIME 10.0 // seconds


//*****************************************************************************
VuFileHostIO::VuFileHostIO()
{
}

//*****************************************************************************
VuFileHostIO::~VuFileHostIO()
{
}

//*****************************************************************************
bool VuFileHostIO::exists(const char *strFileName)
{
	if ( isHostPath(strFileName) )
	{
		strFileName += 5;

		// create request
		VuBinaryDataWriter request = VuDevHostComm::IF()->beginMessage("file");
		request.writeString("exists");
		request.writeString(strFileName);

		if ( VuDevHostComm::IF()->sendMessage(true) )
		{
			VuBinaryDataReader response = VuDevHostComm::IF()->response();
			const char *exists = response.readString();
			if ( strcmp(exists, "true") == 0 )
				return true;
		}
	}

	return false;
}

//*****************************************************************************
VUINT VuFileHostIO::size(const char *strFileName)
{
	if ( isHostPath(strFileName) )
	{
		strFileName += 5;

		// create request
		VuBinaryDataWriter request = VuDevHostComm::IF()->beginMessage("file");
		request.writeString("size");
		request.writeString(strFileName);

		if ( VuDevHostComm::IF()->sendMessage(true) )
		{
			VuBinaryDataReader response = VuDevHostComm::IF()->response();

			VUINT size;
			response.readValueCompat(size);
			return size;
		}
	}

	return -1;
}

//*****************************************************************************
VUHANDLE VuFileHostIO::open(const char *strFileName, VuFile::eMode mode)
{
	if ( isHostPath(strFileName) )
	{
		strFileName += 5;

		// create request
		VuBinaryDataWriter request = VuDevHostComm::IF()->beginMessage("file");
		request.writeString("open");
		request.writeString(strFileName);
		request.writeValueCompat((VUUINT32)mode);

		if ( VuDevHostComm::IF()->sendMessage(true) )
		{
			VuBinaryDataReader response = VuDevHostComm::IF()->response();
			const char *success = response.readString();
			if ( strcmp(success, "true") == 0 )
			{
				VuHostFile *pHostFile = new VuHostFile;
				pHostFile->mName = strFileName;
				pHostFile->mMode = mode;
				response.readValueCompat(pHostFile->mSize);
				pHostFile->mPos = 0;

				return pHostFile;
			}
		}
	}

	return VUNULL;
}

//*****************************************************************************
bool VuFileHostIO::close(VUHANDLE hFile)
{
	VuHostFile *pHostFile = static_cast<VuHostFile *>(hFile);

	delete pHostFile;

	return true;
}

//*****************************************************************************
int VuFileHostIO::read(VUHANDLE hFile, void *pData, VUUINT32 size)
{
	VuHostFile *pHostFile = static_cast<VuHostFile *>(hFile);

	if ( pHostFile->mMode != VuFile::MODE_READ )
		return 0;

	if ( pHostFile->mPos + size > pHostFile->mSize )
		return 0;

	// create request
	VuBinaryDataWriter request = VuDevHostComm::IF()->beginMessage("file");
	request.writeString("read");
	request.writeString(pHostFile->mName);
	request.writeValueCompat(pHostFile->mPos);
	request.writeValueCompat(size);

	if ( !VuDevHostComm::IF()->sendMessage(true) )
		return 0;

	VuBinaryDataReader response = VuDevHostComm::IF()->response();
	if ( (VUUINT32)response.remaining() < size )
		return 0;

	response.readData(pData, size);

	pHostFile->mPos += size;

	return size;
}

//*****************************************************************************
int VuFileHostIO::write(VUHANDLE hFile, const void *pData, VUUINT32 size)
{
	VuHostFile *pHostFile = static_cast<VuHostFile *>(hFile);

	if ( pHostFile->mMode != VuFile::MODE_WRITE )
		return 0;

	// create request
	VuBinaryDataWriter request = VuDevHostComm::IF()->beginMessage("file");
	request.writeString("write");
	request.writeString(pHostFile->mName);
	request.writeValueCompat(pHostFile->mPos);
	request.writeValueCompat(size);

	VU_MEMCPY(request.allocate(size), size, pData, size);

	if ( !VuDevHostComm::IF()->sendMessage(true) )
		return 0;

	VuBinaryDataReader response = VuDevHostComm::IF()->response();
	const char *success = response.readString();
	if ( strcmp(success, "true") != 0 )
		return 0;

	pHostFile->mPos += size;
	pHostFile->mSize = VuMax(pHostFile->mSize, pHostFile->mPos);

	return size;
}

//*****************************************************************************
bool VuFileHostIO::seek(VUHANDLE hFile, int pos)
{
	VuHostFile *pHostFile = static_cast<VuHostFile *>(hFile);

	if ( pos < 0 || pos > (int)pHostFile->mSize )
		return false;

	pHostFile->mPos = pos;

	return true;
}

//*****************************************************************************
int VuFileHostIO::tell(VUHANDLE hFile)
{
	VuHostFile *pHostFile = static_cast<VuHostFile *>(hFile);

	return pHostFile->mPos;
}

//*****************************************************************************
int VuFileHostIO::size(VUHANDLE hFile)
{
	VuHostFile *pHostFile = static_cast<VuHostFile *>(hFile);

	return pHostFile->mSize;
}

//*****************************************************************************
void VuFileHostIO::enumFiles(VuFile::FileList &fileList, const char *strSearchPath, const char *strWildCard)
{
	if ( isHostPath(strSearchPath) )
	{
		strSearchPath += 5;

		// create request
		VuBinaryDataWriter request = VuDevHostComm::IF()->beginMessage("file");
		request.writeString("enumFiles");
		request.writeString(strSearchPath);
		request.writeString(strWildCard);

		if ( VuDevHostComm::IF()->sendMessage(true) )
		{
			VuBinaryDataReader response = VuDevHostComm::IF()->response();
			VUINT32 count;
			response.readValueCompat(count);
			for ( int i = 0; i < count; i++ )
				fileList.push_back(response.readString());
		}
	}
}

//*****************************************************************************
bool VuFileHostIO::modificationTime(const char *strFileName, VUUINT64 &modificationTime)
{
	if ( isHostPath(strFileName) )
	{
		strFileName += 5;

		// create request
		VuBinaryDataWriter request = VuDevHostComm::IF()->beginMessage("file");
		request.writeString("modificationTime");
		request.writeString(strFileName);

		if ( VuDevHostComm::IF()->sendMessage(true) )
		{
			VuBinaryDataReader response = VuDevHostComm::IF()->response();
			const char *success = response.readString();
			if ( strcmp(success, "true") == 0 )
			{
				response.readValueCompat(modificationTime);
				return true;
			}
		}
	}

	return false;
}

//*****************************************************************************
VUUINT32 VuFileHostIO::hash32(const char *strFileName, VUUINT32 hash32)
{
	if ( isHostPath(strFileName) )
	{
		strFileName += 5;

		bool useCache = (hash32 == VU_FNV32_INIT);

		// check cache
		if ( useCache )
		{
			const auto &iter = mHashCache.find(strFileName);
			if ( iter != mHashCache.end() )
			{
				if ( VuSys::IF()->getTime() - iter->second.mSysTime < HASH_CACHE_TIME )
					return iter->second.mHash;
			}
		}


		// create request
		VuBinaryDataWriter request = VuDevHostComm::IF()->beginMessage("file");
		request.writeString("hash32");
		request.writeString(strFileName);
		request.writeValueCompat(hash32);

		if ( VuDevHostComm::IF()->sendMessage(true) )
		{
			VuBinaryDataReader response = VuDevHostComm::IF()->response();
			response.readValueCompat(hash32);

			// store in cache
			if ( useCache )
			{
				HashCacheEntry &entry = mHashCache[strFileName];
				entry.mHash = hash32;
				entry.mSysTime = VuSys::IF()->getTime();
			}

			return hash32;
		}
	}

	return hash32;
}

//*****************************************************************************
bool VuFileHostIO::createDirectory(const char *strPath)
{
	if ( isHostPath(strPath) )
	{
		strPath += 5;

		// create request
		VuBinaryDataWriter request = VuDevHostComm::IF()->beginMessage("file");
		request.writeString("createDirectory");
		request.writeString(strPath);

		if ( VuDevHostComm::IF()->sendMessage(true) )
		{
			VuBinaryDataReader response = VuDevHostComm::IF()->response();
			const char *success = response.readString();
			if ( strcmp(success, "true") == 0 )
				return true;
		}
	}

	return false;
}
