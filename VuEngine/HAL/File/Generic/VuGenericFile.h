//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Generic implementation of File Interface.
// 
//*****************************************************************************

#include "VuEngine/HAL/File/VuFile.h"


class VuGenericFile : public VuFile
{
protected:
	virtual bool				init(const std::string &rootPath, const std::string &projectName);

public:
	virtual bool				exists(const std::string &strFileName);
	virtual VUINT				size(const std::string &strFileName);

	virtual VUHANDLE			open(const std::string &strFileName, eMode mode);
	virtual bool				close(VUHANDLE hFile);
	virtual int					read(VUHANDLE hFile, void *pData, VUINT size);
	virtual int					write(VUHANDLE hFile, const void *pData, VUINT size);
	virtual bool				seek(VUHANDLE hFile, int pos);
	virtual int					tell(VUHANDLE hFile);
	virtual int					size(VUHANDLE hFile);

	virtual bool				modificationTime(const std::string &strFileName, VUUINT64 &modificationTime);

	virtual VUUINT32			hash32(const std::string &strFileName, VUUINT32 hash32);

	virtual bool				remove(const std::string &strFileName);

protected:
	struct VuOpenFile
	{
		VuOpenFile() : mpFile(VUNULL), mHostHandle(VUNULL), mPlatformHandle(VUNULL), mExpansionHandle(VUNULL) {}
		FILE		*mpFile;
		VUHANDLE	mHostHandle;
		VUHANDLE	mPlatformHandle;
		VUHANDLE	mExpansionHandle;
		std::string	mName;
	};
};
