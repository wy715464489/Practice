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

class VuPs4Keyboard : public VuKeyboard
{
public:
	static VuPs4Keyboard *IF() { return static_cast<VuPs4Keyboard *>(VuKeyboard::IF()); }

	void	onKeyDown(VUUINT nChar, VUUINT nRepCnt, VUUINT nFlags) {}
	void	onKeyUp(VUUINT nChar, VUUINT nRepCnt, VUUINT nFlags) {}
};

#else

class VuPs4Keyboard : public VuKeyboard
{
protected:
	virtual bool	init();
	virtual void	release();
	virtual void	tick(float fdt);

public:
	VuPs4Keyboard();

	// platform-specific functionality
	static VuPs4Keyboard *IF() { return static_cast<VuPs4Keyboard *>(VuKeyboard::IF()); }


	// The following should be called by the appropriate Ps4-Style event handler.
	void			onKeysDown(std::vector<VUUINT32>& curList, std::vector<VUUINT32>& prevList);
	void			onKeysUp(std::vector<VUUINT32>& keyList);

private:
	VUUINT32				mKeyMap[256];
	VUINT					mKeyboardHandle;
	std::vector<VUUINT32>	mKeyList[2];
	VUINT					mCurList;
};

#endif // VU_DISABLE_KEYBOARD
