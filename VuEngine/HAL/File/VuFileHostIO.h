//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Handles client host IO.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/File/VuFile.h"


class VuFileHostIO
{
public:
	VuFileHostIO();
	~VuFileHostIO();

	static bool	isHostPath(const char *path) { return strncmp(path, "host:", 5) == 0; }

	bool		exists(const char *strFileName);
	VUINT		size(const char *strFileName);
	VUHANDLE	open(const char *strFileName, VuFile::eMode mode);
	bool		close(VUHANDLE hFile);
	int			read(VUHANDLE hFile, void *pData, VUUINT32 size);
	int			write(VUHANDLE hFile, const void *pData, VUUINT32 size);
	bool		seek(VUHANDLE hFile, int pos);
	int			tell(VUHANDLE hFile);
	int			size(VUHANDLE hFile);

	void		enumFiles(VuFile::FileList &fileList, const char *strSearchPath, const char *strWildCard);
	bool		modificationTime(const char *strFileName, VUUINT64 &modificationTime);
	VUUINT32	hash32(const char *strFileName, VUUINT32 hash32);
	bool		createDirectory(const char *strPath);

private:
	struct VuHostFile
	{
		std::string		mName;
		VuFile::eMode	mMode;
		VUUINT32		mSize;
		VUUINT32		mPos;
	};

	struct HashCacheEntry
	{
		VUUINT32	mHash;
		double		mSysTime;
	};

	typedef std::hash_map<std::string, HashCacheEntry> HashCache;
	HashCache	mHashCache;
};
