//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to the config file.
// 
//*****************************************************************************

#include "VuDevConfig.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Json/VuJsonReader.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuDevConfig, VuDevConfig);


#if !VU_DISABLE_DEV_CONFIG

//*****************************************************************************
bool VuDevConfig::init()
{
	// read dev config file
	{
		const char *strFileName = "DevConfig.json";

		VuJsonReader reader;
		bool bFailure = false;
		if ( VuFile::IF()->exists(VuFile::IF()->getRootPath() + strFileName) )
		{
			if ( !reader.loadFromFile(mConfig, VuFile::IF()->getRootPath() + strFileName) )
				bFailure = true;
		}

		if ( bFailure )
			return VUERROR("Failed to load config file...\n%s", reader.getLastError().c_str());
	}

	return true;
}

#endif // VU_DISABLE_DEV_CONFIG
