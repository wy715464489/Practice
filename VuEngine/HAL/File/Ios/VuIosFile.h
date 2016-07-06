//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ios interface class for File.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/File/Generic/VuGenericFile.h"


class VuIosFile : public VuGenericFile
{
private:
	virtual bool		init(const std::string &rootPath, const std::string &projectName);
	
public:
	
	virtual void		enumFiles(FileList &fileList, const std::string &strSearchPath, const std::string &strWildCard);
	
	virtual bool		createDirectory(const std::string &strPath);
	
	// platform-specific functionality
	static VuIosFile *IF() { return static_cast<VuIosFile *>(VuFile::IF()); }
	
	// files path
	const std::string	&getDocumentPath() { return mDocumentPath; }
	
private:
	std::string			mDocumentPath;
};
