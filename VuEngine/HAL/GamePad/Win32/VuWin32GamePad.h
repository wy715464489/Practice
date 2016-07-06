//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/GamePad/XInput/VuXInput.h"


class VuWin32GamePad : public VuXInput
{
public:
	// platform-specific functionality
	static VuWin32GamePad *IF() { return static_cast<VuWin32GamePad *>(VuGamePad::IF()); }

	void					enable(bool bEnable);

protected:
	virtual eDeviceType		identifyDeviceType(const XINPUT_CAPABILITIES &caps);
};
