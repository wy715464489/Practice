//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/GamePad/XInput/VuXInput.h"


class VuWindowsGamePad : public VuXInput
{
public:
	// platform-specific functionality
	static VuWindowsGamePad *IF() { return static_cast<VuWindowsGamePad *>(VuGamePad::IF()); }

	void					enable(bool bEnable);

protected:
	virtual eDeviceType		identifyDeviceType(const XINPUT_CAPABILITIES &caps);
};
