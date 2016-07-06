//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ios ProfileManager class
// 
//*****************************************************************************

#include "VuEngine/Managers/VuProfileManager.h"
#include "VuEngine/HAL/File/Ios/VuIosFile.h"


class VuIosProfileManager : public VuProfileManager
{
private:
	virtual void	getPath(std::string &path);
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuProfileManager, VuIosProfileManager);


//*****************************************************************************
void VuIosProfileManager::getPath(std::string &path)
{
	path = VuIosFile::IF()->getDocumentPath();

	if ( path.length() && *(path.end() - 1) != '/' )
		path += "/";
}
