//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Windows Config Manager
// 
//*****************************************************************************

#include "VuWindowsConfigManager.h"
#include "VuEngine/HAL/Gfx/D3d11/VuD3d11Gfx.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuConfigManager, VuWindowsConfigManager);


#if defined VUWINSTORE

//*****************************************************************************
bool VuWindowsConfigManager::init(std::string deviceType)
{
	D3D_FEATURE_LEVEL d3dFeatureLevel = VuD3d11Gfx::IF()->getD3dDevice()->GetFeatureLevel();

	if ( d3dFeatureLevel <= D3D_FEATURE_LEVEL_9_1 )
		deviceType = "WinStore Low";
	else if ( d3dFeatureLevel <= D3D_FEATURE_LEVEL_9_3 )
		deviceType = "WinStore Medium";
	else
		deviceType = "WinStore High";

	return VuConfigManager::init(deviceType);
}

#elif defined VUWINPHONE

//*****************************************************************************
bool VuWindowsConfigManager::init(std::string deviceType)
{
	// check for low memory device
	{
		int ramMB;

		// estimate if this device is a low memory device
		std::list<void *> chunks;
		for ( int i = 0; i < 256; i++ )
		{
			void *chunk = malloc(1024*1024);
			if ( chunk )
				chunks.push_back(chunk);
			else
				break;
		}

		ramMB = chunks.size();

		for ( auto chunk : chunks )
			free(chunk);

		mLowMemoryDevice = ramMB < 256;
	}

	SYSTEM_INFO systemInfo;
	GetNativeSystemInfo(&systemInfo);

	D3D_FEATURE_LEVEL d3dFeatureLevel = VuD3d11Gfx::IF()->getD3dDevice()->GetFeatureLevel();

	if ( systemInfo.dwNumberOfProcessors < 2 )
		deviceType = "WinPhone Single Core";
	else if ( mLowMemoryDevice )
		deviceType = "WinPhone Low Memory";
	else if ( systemInfo.dwNumberOfProcessors < 4 )
		deviceType = "WinPhone Dual Core";
	else if ( d3dFeatureLevel <= D3D_FEATURE_LEVEL_9_3 )
		deviceType = "WinPhone Quad Core";
	else
		deviceType = "WinPhone High";

	return VuConfigManager::init(deviceType);
}

#else
	#error Windows platform not defined!
#endif