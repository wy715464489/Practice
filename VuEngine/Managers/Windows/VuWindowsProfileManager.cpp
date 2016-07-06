//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Windows ProfileManager class
// 
//*****************************************************************************

#include "VuWindowsProfileManager.h"
#include "VuEngine/HAL/File/Windows/VuWindowsFile.h"
#include "VuEngine/Util/VuUtf8.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuProfileManager, VuWindowsProfileManager);


//*****************************************************************************
bool VuWindowsProfileManager::init(const std::string &gameName)
{
	return VuProfileManager::init(gameName);
}

//*****************************************************************************
void VuWindowsProfileManager::getPath(std::string &path)
{
	VuWindowsFile::getLocalFolder(path);

	if ( path.length() && *(path.end() - 1) != '/' )
		path += "/";
}
