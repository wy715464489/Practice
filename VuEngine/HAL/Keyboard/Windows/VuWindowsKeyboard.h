//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Windows interface class for Keyboard.
//
//*****************************************************************************

#pragma once

#include "VuEngine/Events/VuEventMap.h"
#include "VuEngine/HAL/Keyboard/VuKeyboard.h"

#if VU_DISABLE_KEYBOARD

class VuWindowsKeyboard : public VuKeyboard
{
public:
	static VuWindowsKeyboard *IF() { return static_cast<VuWindowsKeyboard *>(VuKeyboard::IF()); }

	void	onKeyDown(VUUINT virtualKey) {}
	void	onKeyUp(VUUINT virtualKey) {}
};

#else

class VuWindowsKeyboard : public VuKeyboard
{
	DECLARE_EVENT_MAP

protected:
	virtual bool init();

public:
	VuWindowsKeyboard();

	// platform-specific functionality
	static VuWindowsKeyboard *IF() { return static_cast<VuWindowsKeyboard *>(VuKeyboard::IF()); }

	// The following should be called by the appropriate Windows-Style event handler.
	void		onKeyDown(VUUINT virtualKey);
	void		onKeyUp(VUUINT virtualKey);

private:
	// event handlers
	void		OnWindowsKeyDown(const VuParams &params);
	void		OnWindowsKeyUp(const VuParams &params);

	VUUINT32	mKeyMap[256];
};

#endif // VU_DISABLE_KEYBOARD
