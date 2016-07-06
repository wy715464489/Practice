//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Generic implementation of File Interface.
// 
//*****************************************************************************

#include <sys/stat.h>
#include <errno.h>
#include "VuGenericFile.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/HAL/File/VuFileHostIO.h"


//*****************************************************************************
bool VuGenericFile::init(const std::string &rootPath, const std::string &projectName)
{
	if ( !VuFile::init(rootPath, projectName) )
		return false;

	return true;
}

//*****************************************************************************
bool VuGenericFile::exists(const std::string &strFileName)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strFileName.c_str()) )
		return mpHostIO->exists(strFileName.c_str());
#endif

	// get attributes
	struct stat status;
	if ( stat(strFileName.c_str(), &status) == -1 )
		return false;
	
	return true;
}

//*****************************************************************************
VUINT VuGenericFile::size(const std::string &strFileName)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strFileName.c_str()) )
		return mpHostIO->size(strFileName.c_str());
#endif

	// get attributes
	struct stat status;
	if ( stat(strFileName.c_str(), &status) == -1 )
		return -1;

	return status.st_size;
}

//*****************************************************************************
VUHANDLE VuGenericFile::open(const std::string &strFileName, eMode mode)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strFileName.c_str()) )
	{
		VUHANDLE hHostHandle = mpHostIO->open(strFileName.c_str(), mode);
		if ( hHostHandle == VUNULL )
			return VUNULL;
		
		VuOpenFile *pOpenFile = new VuOpenFile;
		pOpenFile->mHostHandle = hHostHandle;
		pOpenFile->mName = strFileName;

		return pOpenFile;
	}
#endif

	FILE *fp = VUNULL;
	if ( mode == MODE_READ )
	{
		fopen_s(&fp, strFileName.c_str(), "rb");
	}
	else if ( mode == MODE_WRITE )
	{
		fopen_s(&fp, strFileName.c_str(), "wb");
	}
	else if ( mode == MODE_READ_WRITE )
	{
		fopen_s(&fp, strFileName.c_str(), "rb+");
	}

	if ( !fp )
	{
		return VUNULL;
	}

	VuOpenFile *pOpenFile = new VuOpenFile;
	pOpenFile->mpFile = fp;
	pOpenFile->mName = strFileName;

	return pOpenFile;
}

//*****************************************************************************
bool VuGenericFile::close(VUHANDLE hFile)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

#if !VU_DISABLE_DEV_HOST_COMM
	if ( pOpenFile->mHostHandle )
	{
		VUHANDLE hHostHandle = pOpenFile->mHostHandle;
		delete pOpenFile;
		return mpHostIO->close(hHostHandle);
	}
#endif

	FILE *fp = pOpenFile->mpFile;
	delete pOpenFile;

	return fclose(fp) == 0;
}

//*****************************************************************************
int VuGenericFile::read(VUHANDLE hFile, void *pData, VUINT size)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

#if !VU_DISABLE_DEV_HOST_COMM
	if ( pOpenFile->mHostHandle )
		return mpHostIO->read(pOpenFile->mHostHandle, pData, size);
#endif

	return (int)fread(pData, 1, size, pOpenFile->mpFile);
}

//*****************************************************************************
int VuGenericFile::write(VUHANDLE hFile, const void *pData, VUINT size)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

#if !VU_DISABLE_DEV_HOST_COMM
	if ( pOpenFile->mHostHandle )
		return mpHostIO->write(pOpenFile->mHostHandle, pData, size);
#endif

	return (int)fwrite(pData, 1, size, pOpenFile->mpFile);
}

//*****************************************************************************
bool VuGenericFile::seek(VUHANDLE hFile, int pos)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

#if !VU_DISABLE_DEV_HOST_COMM
	if ( pOpenFile->mHostHandle )
		return mpHostIO->seek(pOpenFile->mHostHandle, pos);
#endif

	if ( fseek(pOpenFile->mpFile, pos, SEEK_SET) == -1 )
		return false;

	return true;
}

//*****************************************************************************
int VuGenericFile::tell(VUHANDLE hFile)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

#if !VU_DISABLE_DEV_HOST_COMM
	if ( pOpenFile->mHostHandle )
		return mpHostIO->tell(pOpenFile->mHostHandle);
#endif

	return ftell(pOpenFile->mpFile);
}

//*****************************************************************************
int VuGenericFile::size(VUHANDLE hFile)
{
	VuOpenFile *pOpenFile = static_cast<VuOpenFile *>(hFile);

#if !VU_DISABLE_DEV_HOST_COMM
	if ( pOpenFile->mHostHandle )
		return mpHostIO->size(pOpenFile->mHostHandle);
#endif

	struct stat status;
#ifdef VUBB10
	if ( fstat(pOpenFile->mpFile->_Handle, &status) == -1 )
#else
	if ( fstat(pOpenFile->mpFile->_file, &status) == -1 )
#endif
		return 0;

	return status.st_size;
}

//*****************************************************************************
bool VuGenericFile::modificationTime(const std::string &strFileName, VUUINT64 &modificationTime)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strFileName.c_str()) )
		return mpHostIO->modificationTime(strFileName.c_str(), modificationTime);
#endif

	// get attributes
	struct stat status;
	if ( stat(strFileName.c_str(), &status) == -1 )
		return 0;

	modificationTime = status.st_mtime;

	return true;
}

//*****************************************************************************
VUUINT32 VuGenericFile::hash32(const std::string &strFileName, VUUINT32 hash32)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strFileName.c_str()) )
		return mpHostIO->hash32(strFileName.c_str(), hash32);
#endif

	// open file
	FILE *fp = VUNULL;
	fopen_s(&fp, strFileName.c_str(), "rb");
	if ( fp )
	{
		// get size
		struct stat status;
		if ( fstat(fp->_file, &status) == 0 )
		{
			VuArray<VUBYTE> data(0);
			data.resize(status.st_size);

			// read file
			if ( fread(&data.begin(), 1, data.size(), fp) == data.size() )
			{
				hash32 = VuHash::fnv32(&data.begin(), data.size(), hash32);
			}
		}

		// close file
		fclose(fp);
	}

	return hash32;
}

//*****************************************************************************
bool VuGenericFile::remove(const std::string &strFileName)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strFileName.c_str()) )
		VUASSERT(0, "Host file deletion not supported yet!");
#endif

	return ::remove(strFileName.c_str()) == 0;
}