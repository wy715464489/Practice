//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Windows ProfileManager class
// 
//*****************************************************************************

#include "VuEngine/Managers/VuProfileManager.h"


class VuWindowsProfileManager : public VuProfileManager
{
public:
	virtual bool	init(const std::string &gameName);

private:
	virtual void	getPath(std::string &path);
};
