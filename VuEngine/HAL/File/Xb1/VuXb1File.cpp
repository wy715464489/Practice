//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to File library.
// 
//*****************************************************************************

#include <sys/stat.h>
#include "VuXb1File.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/Util/VuUtf8.h"
#include "VuEngine/Dev/VuDevHostComm.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuFile, VuXb1File);


//*****************************************************************************
bool VuXb1File::init(const std::string &rootPath, const std::string &projectName)
{
	if ( !VuGenericWinFile::init(rootPath, projectName) )
		return false;

	if ( projectName.length() )
	{
		mCachePath = "D:/" + projectName;

		// does directory already exist?
		struct stat status;
		if ( stat(mCachePath.c_str(), &status) == -1 )
		{
			// create directory
			wchar_t wpath[MAX_PATH];
			VuUtf8::convertUtf8StringToWCharString(mCachePath.c_str(), wpath, MAX_PATH);
			if ( !CreateDirectoryW(wpath, VUNULL) )
				return false;
		}

		mCachePath += "/";
	}

	return true;
}
