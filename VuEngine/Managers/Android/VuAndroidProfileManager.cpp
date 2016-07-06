//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android ProfileManager class
// 
//*****************************************************************************

#include "VuEngine/Managers/VuProfileManager.h"
#include "VuEngine/HAL/File/Android/VuAndroidFile.h"


class VuAndroidProfileManager : public VuProfileManager
{
private:
	virtual void	getPath(std::string &path);
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuProfileManager, VuAndroidProfileManager);


//*****************************************************************************
void VuAndroidProfileManager::getPath(std::string &path)
{
	VuAndroidFile::IF()->getFilesPath(path);

	if ( path.length() && *(path.end() - 1) != '/' )
		path += "/";
}
