//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android interface class for Keyboard.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Keyboard/VuKeyboard.h"

#if VU_DISABLE_KEYBOARD

class VuAndroidKeyboard : public VuKeyboard
{
public:
	static VuAndroidKeyboard *IF() { return static_cast<VuAndroidKeyboard *>(VuKeyboard::IF()); }

	void			onKeyDown(VUUINT32 code) {}
	void			onKeyUp(VUUINT32 code) {}
};

#else

class VuAndroidKeyboard : public VuKeyboard
{
protected:
	virtual bool init();

public:
	VuAndroidKeyboard();

	// platform-specific functionality
	static VuAndroidKeyboard *IF() { return static_cast<VuAndroidKeyboard *>(VuKeyboard::IF()); }

	// The following should be called by the appropriate windows procedure.
	void			onKeyDown(VUUINT32 code);
	void			onKeyUp(VUUINT32 code);

private:
	char			translateKey(VUUINT nChar);

	VUUINT32		mKeyMap[256];
};


#endif // VU_DISABLE_KEYBOARD
