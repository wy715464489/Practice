//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Generic windows implementation of File Interface.
// 
//*****************************************************************************

#include "VuEngine/HAL/File/Generic/VuGenericFile.h"


class VuGenericWinFile : public VuGenericFile
{
public:
	virtual bool				exists(const std::string &strFileName);
	virtual VUINT				size(const std::string &strFileName);

	virtual VUHANDLE			open(const std::string &strFileName, eMode mode);

	virtual void				enumFiles(FileList &fileList, const std::string &strSearchPath, const std::string &strWildCard);

	virtual bool				modificationTime(const std::string &strFileName, VUUINT64 &modificationTime);

	virtual VUUINT32			hash32(const std::string &strFileName, VUUINT32 hash32);

	virtual bool				createDirectory(const std::string &strPath);

	virtual bool				remove(const std::string &strFileName);
};
