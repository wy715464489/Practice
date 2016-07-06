//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to File library.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Util/VuHash.h"

class VuEngine;
class VuFileHostIO;


class VuFile : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuFile)

protected:

	VuFile();

	// called by engine
	friend class VuEngine;
	virtual bool init(const std::string &rootPath, const std::string &projectName);
	virtual void release();

public:

	const std::string	&getRootPath() { return mRootPath; }
	const std::string	&getCachePath()	{ return mCachePath; }

	virtual bool				exists(const std::string &strFileName) = 0;
	virtual VUINT				size(const std::string &strFileName) = 0;

	// i/o
	enum eMode { MODE_READ, MODE_WRITE, MODE_READ_WRITE };
	virtual VUHANDLE			open(const std::string &strFileName, eMode mode) = 0;
	virtual bool				close(VUHANDLE hFile) = 0;
	virtual int					read(VUHANDLE hFile, void *pData, VUINT size) = 0;
	virtual int					write(VUHANDLE hFile, const void *pData, VUINT size) = 0;
	virtual bool				seek(VUHANDLE hFile, int pos) = 0;
	virtual int					tell(VUHANDLE hFile) = 0;
	virtual int					size(VUHANDLE hFile) = 0;

	// file enumeration
								typedef std::list<std::string> FileList;
	virtual void				enumFiles(FileList &fileList, const std::string &strSearchPath, const std::string &strWildCard = "*.*") = 0;

	// file time
	virtual bool				modificationTime(const std::string &strFileName, VUUINT64 &modificationTime) = 0;

	// file hash
	virtual VUUINT32			hash32(const std::string &strFileName, VUUINT32 hash32 = VU_FNV32_INIT) = 0;

	// create directory
	virtual bool				createDirectory(const std::string &strPath) = 0;

	// delete
	virtual bool				remove(const std::string &strFileName) = 0;

protected:
	std::string					mRootPath;
	std::string					mCachePath;
	VuFileHostIO				*mpHostIO;
};