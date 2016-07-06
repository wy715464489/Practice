//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//*****************************************************************************

#include "VuWin32GamePad.h"


#if defined VUWIN32
	#define LEGACY_XINPUT
#endif

// libs
#if defined LEGACY_XINPUT
	#pragma comment(lib, "xinput9_1_0.lib")
#elif defined VUWINSTORE
	#pragma comment(lib, "xinput.lib")
#endif


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuGamePad, VuWin32GamePad);


//*****************************************************************************
void VuWin32GamePad::enable(bool bEnable)
{
#ifndef LEGACY_XINPUT
	XInputEnable(bEnable);
#endif
}

//*****************************************************************************
VuGamePad::eDeviceType VuWin32GamePad::identifyDeviceType(const XINPUT_CAPABILITIES &caps)
{
	eDeviceType type = DEVICE_UNKNOWN;

	switch(caps.SubType)
	{
/* does not exist in Win32 XInput.h
	case XINPUT_DEVSUBTYPE_UNKNOWN:
		VUPRINTF("Device is an unknown subtype.\n");
		break;
*/
	case XINPUT_DEVSUBTYPE_GAMEPAD:
		VUPRINTF("Device is a gamepad.\n");
		type = DEVICE_GAMEPAD;
		break;

	case XINPUT_DEVSUBTYPE_WHEEL:
		VUPRINTF("Device is a wheel.\n");
		type = DEVICE_STEERING_WHEEL;
		break;

	case XINPUT_DEVSUBTYPE_ARCADE_STICK:
		VUPRINTF("Device is an arcade stick.\n");
		break;

	case XINPUT_DEVSUBTYPE_FLIGHT_STICK:
		VUPRINTF("Device is a flight stick.\n");
		break;

/* does not exist in Win32 XInput.h
	case XINPUT_DEVSUBTYPE_DANCEPAD:
		VUPRINTF("Device is a dance pad.\n");
		break;
*/
	case XINPUT_DEVSUBTYPE_GUITAR:
		VUPRINTF("Device is a guitar.\n");
		break;

/* does not exist in Win32 XInput.h
	case XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE:
		VUPRINTF("Device is a guitar alternate.\n");
		break;
*/

	case XINPUT_DEVSUBTYPE_DRUM_KIT:
		VUPRINTF("Device is a drum kit.\n");
		break;

	default:
		VUPRINTF("Device is unknown.\n");
		break;
	}

	return type;
}
