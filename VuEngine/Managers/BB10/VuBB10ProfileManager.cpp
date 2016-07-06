//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  BB10 ProfileManager class
// 
//*****************************************************************************

#include "VuEngine/Managers/VuProfileManager.h"
#include "VuEngine/HAL/File/BB10/VuBB10File.h"


class VuBB10ProfileManager : public VuProfileManager
{
private:
	virtual void	getPath(std::string &path);
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuProfileManager, VuBB10ProfileManager);


//*****************************************************************************
void VuBB10ProfileManager::getPath(std::string &path)
{
	path = VuBB10File::IF()->getDataPath();
}
