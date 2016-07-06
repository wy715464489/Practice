//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to File library.
// 
//*****************************************************************************

#include <shlobj.h>
#include "VuWin32File.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/Dev/VuDevHostComm.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuFile, VuWin32File);


//*****************************************************************************
bool VuWin32File::init(const std::string &rootPath, const std::string &projectName)
{
	if ( !VuGenericWinFile::init(rootPath, projectName) )
		return false;

	if ( projectName.length() )
	{
		std::string subFolder = "Vector Unit\\" + projectName;

		char strPath[MAX_PATH];
		if ( SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, subFolder.c_str(), strPath) != S_OK )
			return VUWARNING("VuFileImpl::init() Unable to set cache path.");

		mCachePath = VuFileUtil::fixSlashes(strPath);
		if ( mCachePath.length() && *(mCachePath.end() - 1) != '/' )
			mCachePath += "/";
	}

	return true;
}
