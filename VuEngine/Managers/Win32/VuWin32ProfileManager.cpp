//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 ProfileManager class
// 
//*****************************************************************************

#include <shlobj.h>
#include "VuEngine/Managers/VuProfileManager.h"


class VuWin32ProfileManager : public VuProfileManager
{
public:
	virtual bool	init(const std::string &gameName);

private:
	virtual void	getPath(std::string &path) { path = mPath; }

	char			mPath[MAX_PATH];
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuProfileManager, VuWin32ProfileManager);


//*****************************************************************************
bool VuWin32ProfileManager::init(const std::string &gameName)
{
	std::string subFolder = "Vector Unit\\" + gameName;
	if (SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, subFolder.c_str(), mPath) != S_OK)
		return VUWARNING("VuWin32ProfileManager::loadInternal() Unable to create profile path.");

	VU_STRCAT(mPath, sizeof(mPath), "/");

	return VuProfileManager::init(gameName);
}
