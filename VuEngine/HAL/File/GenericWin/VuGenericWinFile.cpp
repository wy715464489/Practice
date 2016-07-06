//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Generic windows implementation of File Interface.
// 
//*****************************************************************************

#include <sys/stat.h>
#include "VuGenericWinFile.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/Util/VuUtf8.h"
#include "VuEngine/HAL/File/VuFileHostIO.h"


//*****************************************************************************
bool VuGenericWinFile::exists(const std::string &strFileName)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strFileName.c_str()) )
		return mpHostIO->exists(strFileName.c_str());
#endif

	wchar_t wpath[MAX_PATH];
	VuUtf8::convertUtf8StringToWCharString(strFileName.c_str(), wpath, MAX_PATH);

	// get attributes
	struct _stat status;
	if ( _wstat(wpath, &status) == -1 )
		return false;
	
	return true;
}

//*****************************************************************************
VUINT VuGenericWinFile::size(const std::string &strFileName)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strFileName.c_str()) )
		return mpHostIO->size(strFileName.c_str());
#endif

	wchar_t wpath[MAX_PATH];
	VuUtf8::convertUtf8StringToWCharString(strFileName.c_str(), wpath, MAX_PATH);

	// get attributes
	struct _stat status;
	if ( _wstat(wpath, &status) == -1 )
		return -1;

	return status.st_size;
}

//*****************************************************************************
VUHANDLE VuGenericWinFile::open(const std::string &strFileName, eMode mode)
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

	wchar_t wpath[MAX_PATH];
	VuUtf8::convertUtf8StringToWCharString(strFileName.c_str(), wpath, MAX_PATH);

	FILE *fp = VUNULL;
	if ( mode == MODE_READ )
	{
		_wfopen_s(&fp, wpath, L"rb");
	}
	else if ( mode == MODE_WRITE )
	{
		_wfopen_s(&fp, wpath, L"wb");
	}
	else if ( mode == MODE_READ_WRITE )
	{
		_wfopen_s(&fp, wpath, L"rb+");
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
void VuGenericWinFile::enumFiles(VuFile::FileList &fileList, const std::string &strSearchPath, const std::string &strWildCard)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strSearchPath.c_str()) )
		mpHostIO->enumFiles(fileList, strSearchPath.c_str(), strWildCard.c_str());
#endif

	// build search path
	std::string strSearch = VuFileUtil::fixSlashes(strSearchPath + "/" + strWildCard);

	WIN32_FIND_DATAW fd;
	wchar_t wpath[MAX_PATH];
	VuUtf8::convertUtf8StringToWCharString(strSearch.c_str(), wpath, MAX_PATH);
	HANDLE hSearch = FindFirstFileExW(wpath, FindExInfoBasic, &fd, FindExSearchNameMatch, NULL, 0);
	if ( hSearch == INVALID_HANDLE_VALUE )
		return;

	do
	{
		if ( fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )
			continue;

		if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			if ( fd.cFileName[0] == '.' )
				continue;

		std::string fileName;
		VuUtf8::convertWCharStringToUtf8String(fd.cFileName, fileName);
		fileList.push_back(fileName);
	}
	while ( FindNextFileW(hSearch, &fd) );

	FindClose(hSearch);
}

//*****************************************************************************
bool VuGenericWinFile::modificationTime(const std::string &strFileName, VUUINT64 &modificationTime)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strFileName.c_str()) )
		return mpHostIO->modificationTime(strFileName.c_str(), modificationTime);
#endif

	wchar_t wpath[MAX_PATH];
	VuUtf8::convertUtf8StringToWCharString(strFileName.c_str(), wpath, MAX_PATH);

	// get attributes
	struct _stat status;
	if ( _wstat(wpath, &status) == -1 )
		return 0;

	modificationTime = status.st_mtime;

	return true;
}

//*****************************************************************************
VUUINT32 VuGenericWinFile::hash32(const std::string &strFileName, VUUINT32 hash32)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strFileName.c_str()) )
		return mpHostIO->hash32(strFileName.c_str(), hash32);
#endif

	wchar_t wpath[MAX_PATH];
	VuUtf8::convertUtf8StringToWCharString(strFileName.c_str(), wpath, MAX_PATH);

	// open file
	FILE *fp = VUNULL;
	_wfopen_s(&fp, wpath, L"rb");
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
bool VuGenericWinFile::createDirectory(const std::string &strPath)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strPath.c_str()) )
		return mpHostIO->createDirectory(strPath.c_str());
#endif

	std::string cleanPath = VuFileUtil::fixSlashes(strPath);
	while ( cleanPath.length() && cleanPath[cleanPath.length() - 1] == '/' )
		cleanPath.resize(cleanPath.length() - 1);

	if ( exists(strPath) )
		return true;

	std::string parentPath = VuFileUtil::getPath(strPath);
	if ( parentPath.length() )
		if ( !createDirectory(parentPath) )
			return false;

	wchar_t wpath[MAX_PATH];
	VuUtf8::convertUtf8StringToWCharString(strPath.c_str(), wpath, MAX_PATH);
	if ( !CreateDirectoryW(wpath, VUNULL) )
		return false;

	return true;
}

//*****************************************************************************
bool VuGenericWinFile::remove(const std::string &strFileName)
{
#if !VU_DISABLE_DEV_HOST_COMM
	if ( mpHostIO && mpHostIO->isHostPath(strFileName.c_str()) )
		VUASSERT(0, "Host file deletion not supported yet!");
#endif

	wchar_t wpath[MAX_PATH];
	VuUtf8::convertUtf8StringToWCharString(strFileName.c_str(), wpath, MAX_PATH);

	return _wremove(wpath) == 0;
}