//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Xb1 interface class for Keyboard.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Keyboard/VuKeyboard.h"

#if VU_DISABLE_KEYBOARD

class VuXb1Keyboard : public VuKeyboard
{
public:
	static VuXb1Keyboard *IF() { return static_cast<VuXb1Keyboard *>(VuKeyboard::IF()); }

	void	onKeyDown(VUUINT virtualKey) {}
	void	onKeyUp(VUUINT virtualKey) {}
};

#else

class VuXb1Keyboard : public VuKeyboard
{
protected:
	virtual bool init();

public:
	VuXb1Keyboard();

	// platform-specific functionality
	static VuXb1Keyboard *IF() { return static_cast<VuXb1Keyboard *>(VuKeyboard::IF()); }

	// The following should be called by the appropriate Xb1-Style event handler.
	void			onKeyDown(VUUINT virtualKey);
	void			onKeyUp(VUUINT virtualKey);

private:
	VUUINT32		mKeyMap[256];
};

#endif // VU_DISABLE_KEYBOARD
