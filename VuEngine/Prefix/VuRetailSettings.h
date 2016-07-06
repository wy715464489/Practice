//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Configures settings for retail configuration.
// 
//*****************************************************************************

#pragma once


#define VU_DISABLE_ASSERTS 1
#define VU_DISABLE_CHEATS 1
#define VU_DISABLE_DEV 1
#define VU_DISABLE_DEV_KEYBOARD 1
#define VU_DISABLE_DEV_CONFIG 1
#define VU_DISABLE_DEV_CONSOLE 1
#define VU_DISABLE_DEV_HOST_COMM 1
#define VU_DISABLE_DEV_MENU 1
#define VU_DISABLE_DEV_PROFILE 1
#define VU_DISABLE_DEV_STAT 1
#define VU_DISABLE_DEV_TIMER 1
#define VU_DISABLE_DEV_UTIL 1
#define VU_DISABLE_DEBUG_OUTPUT 1
#define VU_DISABLE_VERTEX_CACHE_OPT 1
#define VU_DISABLE_TELEMETRY 1
#define VU_DISABLE_BAKING 1
#define VU_DISABLE_TEST_ADS 1


#if VU_DISABLE_DEBUG_OUTPUT

	#define VUPRINTF(x, ...)
	#define VUWPRINTF(x, ...)
	#define VUPRINT(x)
	#define VUERROR(x, ...)		false
	#define VUWARNING(x, ...)	false

#else

	#define VUPRINTF(x, ...)	VuSys::IF()->printf(x, ##__VA_ARGS__)
	#define VUWPRINTF(x, ...)	VuSys::IF()->wprintf(x, ##__VA_ARGS__)
	#define VUPRINT(x)			VuSys::IF()->print(x)
	#define VUERROR(x, ...)		VuSys::IF()->exitWithError(x, ##__VA_ARGS__)
	#define VUWARNING(x, ...)	VuSys::IF()->warning(x, ##__VA_ARGS__)

#endif // VU_DISABLE_DEBUG_OUTPUT


#if defined (VUWIN32) || defined (VUXBOX360)

	#pragma warning(disable : 4101 4390)

#endif