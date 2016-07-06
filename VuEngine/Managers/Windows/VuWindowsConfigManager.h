//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Windows Config Manager
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Managers/VuConfigManager.h"


class VuWindowsConfigManager : public VuConfigManager
{
public:
	VuWindowsConfigManager() : mLowMemoryDevice(false) {}

	virtual bool	init(std::string deviceType);

	// platform-specific functionality
	static VuWindowsConfigManager *IF() { return static_cast<VuWindowsConfigManager *>(VuConfigManager::IF()); }

	bool			isLowMemoryDevice() { return mLowMemoryDevice; }

private:
	bool			mLowMemoryDevice;
};
