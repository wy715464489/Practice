//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class for Keyboard.
//
//*****************************************************************************

#include "VuWin32Keyboard.h"


#if VU_DISABLE_KEYBOARD

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuKeyboard, VuKeyboard);

#else

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuKeyboard, VuWin32Keyboard);


//*****************************************************************************
VuWin32Keyboard::VuWin32Keyboard()
{
	memset(mKeyMap, 0, sizeof(mKeyMap));
}

//*****************************************************************************
bool VuWin32Keyboard::init()
{
	if ( !VuKeyboard::init() )
		return false;

	mKeyMap['0']	= VUKEY_0;
	mKeyMap['1']	= VUKEY_1;
	mKeyMap['2']	= VUKEY_2;
	mKeyMap['3']	= VUKEY_3;
	mKeyMap['4']	= VUKEY_4;
	mKeyMap['5']	= VUKEY_5;
	mKeyMap['6']	= VUKEY_6;
	mKeyMap['7']	= VUKEY_7;
	mKeyMap['8']	= VUKEY_8;
	mKeyMap['9']	= VUKEY_9;

	mKeyMap['A']	= VUKEY_A;
	mKeyMap['B']	= VUKEY_B;
	mKeyMap['C']	= VUKEY_C;
	mKeyMap['D']	= VUKEY_D;
	mKeyMap['E']	= VUKEY_E;
	mKeyMap['F']	= VUKEY_F;
	mKeyMap['G']	= VUKEY_G;
	mKeyMap['H']	= VUKEY_H;
	mKeyMap['I']	= VUKEY_I;
	mKeyMap['J']	= VUKEY_J;
	mKeyMap['K']	= VUKEY_K;
	mKeyMap['L']	= VUKEY_L;
	mKeyMap['M']	= VUKEY_M;
	mKeyMap['N']	= VUKEY_N;
	mKeyMap['O']	= VUKEY_O;
	mKeyMap['P']	= VUKEY_P;
	mKeyMap['Q']	= VUKEY_Q;
	mKeyMap['R']	= VUKEY_R;
	mKeyMap['S']	= VUKEY_S;
	mKeyMap['T']	= VUKEY_T;
	mKeyMap['U']	= VUKEY_U;
	mKeyMap['V']	= VUKEY_V;
	mKeyMap['W']	= VUKEY_W;
	mKeyMap['X']	= VUKEY_X;
	mKeyMap['Y']	= VUKEY_Y;
	mKeyMap['Z']	= VUKEY_Z;

	mKeyMap[VK_BACK]		= VUKEY_BACK;
	mKeyMap[VK_RETURN]		= VUKEY_ENTER;
	mKeyMap[VK_SPACE]		= VUKEY_SPACE;
	mKeyMap[VK_TAB]			= VUKEY_TAB;
	mKeyMap[VK_OEM_COMMA]	= VUKEY_COMMA;
	mKeyMap[VK_OEM_PERIOD]	= VUKEY_PERIOD;
	mKeyMap[VK_OEM_MINUS]	= VUKEY_MINUS;
	mKeyMap[VK_OEM_PLUS]	= VUKEY_PLUS;
	mKeyMap[VK_OEM_7]		= VUKEY_APOSTROPHE;
	mKeyMap[VK_OEM_1]		= VUKEY_SEMICOLON;
	mKeyMap[VK_OEM_2]		= VUKEY_SLASH;
	mKeyMap[VK_OEM_3]		= VUKEY_GRAVE;
	mKeyMap[VK_OEM_5]		= VUKEY_BACKSLASH;
	mKeyMap[VK_OEM_4]		= VUKEY_LEFT_BRACKET;
	mKeyMap[VK_OEM_6]		= VUKEY_RIGHT_BRACKET;

	mKeyMap[VK_LEFT]		= VUKEY_LEFT;
	mKeyMap[VK_RIGHT]		= VUKEY_RIGHT;
	mKeyMap[VK_UP]			= VUKEY_UP;
	mKeyMap[VK_DOWN]		= VUKEY_DOWN;

	mKeyMap[VK_SHIFT]		= VUKEY_SHIFT;
	mKeyMap[VK_MENU]		= VUKEY_ALT;

	mKeyMap[VK_CONTROL]		= VUKEY_CONTROL;
	mKeyMap[VK_ESCAPE]		= VUKEY_ESCAPE;
	mKeyMap[VK_SCROLL]		= VUKEY_SCROLL;
	mKeyMap[VK_PAUSE]		= VUKEY_PAUSE;
	mKeyMap[VK_INSERT]		= VUKEY_INSERT;
	mKeyMap[VK_DELETE]		= VUKEY_DELETE;
	mKeyMap[VK_HOME]		= VUKEY_HOME;
	mKeyMap[VK_END]			= VUKEY_END;
	mKeyMap[VK_PRIOR]		= VUKEY_PRIOR;
	mKeyMap[VK_NEXT]		= VUKEY_NEXT;
	mKeyMap[VK_F1]			= VUKEY_F1;
	mKeyMap[VK_F2]			= VUKEY_F2;
	mKeyMap[VK_F3]			= VUKEY_F3;
	mKeyMap[VK_F4]			= VUKEY_F4;
	mKeyMap[VK_F5]			= VUKEY_F5;
	mKeyMap[VK_F6]			= VUKEY_F6;
	mKeyMap[VK_F7]			= VUKEY_F7;
	mKeyMap[VK_F8]			= VUKEY_F8;
	mKeyMap[VK_F9]			= VUKEY_F9;
	mKeyMap[VK_F10]			= VUKEY_F10;
	mKeyMap[VK_F11]			= VUKEY_F11;
	mKeyMap[VK_F12]			= VUKEY_F12;

	return true;
}

//*****************************************************************************
void VuWin32Keyboard::onKeyDown(VUUINT nChar, VUUINT nRepCnt, VUUINT nFlags)
{
	if ( !(nFlags & (1<<14)) )
	{
		VUUINT32 key = mKeyMap[nChar];
		if ( key )
		{
			onKeyDownInternal(key);
		}
	}
}

//*****************************************************************************
void VuWin32Keyboard::onKeyUp(VUUINT nChar, VUUINT nRepCnt, VUUINT nFlags)
{
	VUUINT32 key = mKeyMap[nChar];
	if ( key )
	{
		onKeyUpInternal(key);
	}
}

#endif // VU_DISABLE_KEYBOARD
