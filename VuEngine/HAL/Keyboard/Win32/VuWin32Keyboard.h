//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 interface class for Keyboard.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Keyboard/VuKeyboard.h"

#if VU_DISABLE_KEYBOARD

class VuWin32Keyboard : public VuKeyboard
{
public:
	static VuWin32Keyboard *IF() { return static_cast<VuWin32Keyboard *>(VuKeyboard::IF()); }

	void	onKeyDown(VUUINT nChar, VUUINT nRepCnt, VUUINT nFlags) {}
	void	onKeyUp(VUUINT nChar, VUUINT nRepCnt, VUUINT nFlags) {}
};

#else

class VuWin32Keyboard : public VuKeyboard
{
protected:
	virtual bool init();

public:
	VuWin32Keyboard();

	// platform-specific functionality
	static VuWin32Keyboard *IF() { return static_cast<VuWin32Keyboard *>(VuKeyboard::IF()); }

	// For Win32, it's much more elegant to simply intercept
	// the windows keyboard functionality than it is to use
	// DirectInput.  The following should be called by the
	// appropriate windows procedure (MFC or Win32).
	void			onKeyDown(VUUINT nChar, VUUINT nRepCnt, VUUINT nFlags);
	void			onKeyUp(VUUINT nChar, VUUINT nRepCnt, VUUINT nFlags);

private:
	VUUINT32		mKeyMap[256];
};

#endif // VU_DISABLE_KEYBOARD
