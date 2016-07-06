//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 Config Manager
// 
//*****************************************************************************

#include "VuEngine/Managers/VuConfigManager.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11Gfx.h"


class VuWin32ConfigManager : public VuConfigManager
{
public:
	virtual bool	init(std::string deviceType);
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuConfigManager, VuWin32ConfigManager);


//*****************************************************************************
bool VuWin32ConfigManager::init(std::string deviceType)
{
	if (VuD3d11Gfx::IF())
	{
		D3D_FEATURE_LEVEL d3dFeatureLevel = VuD3d11Gfx::IF()->getD3dDevice()->GetFeatureLevel();

		if (d3dFeatureLevel <= D3D_FEATURE_LEVEL_9_3)
			deviceType = "Win32 Low";
		else
			deviceType = "Win32 High";
	}

	return VuConfigManager::init(deviceType);
}