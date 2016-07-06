//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class for Keyboard.
//
//*****************************************************************************

#include <android/keycodes.h>
#include "VuAndroidKeyboard.h"


#if VU_DISABLE_KEYBOARD

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuKeyboard, VuKeyboard);

#else

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuKeyboard, VuAndroidKeyboard);


//*****************************************************************************
VuAndroidKeyboard::VuAndroidKeyboard()
{
	memset(mKeyMap, 0, sizeof(mKeyMap));
}

//*****************************************************************************
bool VuAndroidKeyboard::init()
{
	if ( !VuKeyboard::init() )
		return false;

	mKeyMap[AKEYCODE_0] = VUKEY_0;
	mKeyMap[AKEYCODE_1] = VUKEY_1;
	mKeyMap[AKEYCODE_2] = VUKEY_2;
	mKeyMap[AKEYCODE_3] = VUKEY_3;
	mKeyMap[AKEYCODE_4] = VUKEY_4;
	mKeyMap[AKEYCODE_5] = VUKEY_5;
	mKeyMap[AKEYCODE_6] = VUKEY_6;
	mKeyMap[AKEYCODE_7] = VUKEY_7;
	mKeyMap[AKEYCODE_8] = VUKEY_8;
	mKeyMap[AKEYCODE_9] = VUKEY_9;
					    
	mKeyMap[AKEYCODE_A] = VUKEY_A;
	mKeyMap[AKEYCODE_B] = VUKEY_B;
	mKeyMap[AKEYCODE_C] = VUKEY_C;
	mKeyMap[AKEYCODE_D] = VUKEY_D;
	mKeyMap[AKEYCODE_E] = VUKEY_E;
	mKeyMap[AKEYCODE_F] = VUKEY_F;
	mKeyMap[AKEYCODE_G] = VUKEY_G;
	mKeyMap[AKEYCODE_H] = VUKEY_H;
	mKeyMap[AKEYCODE_I] = VUKEY_I;
	mKeyMap[AKEYCODE_J] = VUKEY_J;
	mKeyMap[AKEYCODE_K] = VUKEY_K;
	mKeyMap[AKEYCODE_L] = VUKEY_L;
	mKeyMap[AKEYCODE_M] = VUKEY_M;
	mKeyMap[AKEYCODE_N] = VUKEY_N;
	mKeyMap[AKEYCODE_O] = VUKEY_O;
	mKeyMap[AKEYCODE_P] = VUKEY_P;
	mKeyMap[AKEYCODE_Q] = VUKEY_Q;
	mKeyMap[AKEYCODE_R] = VUKEY_R;
	mKeyMap[AKEYCODE_S] = VUKEY_S;
	mKeyMap[AKEYCODE_T] = VUKEY_T;
	mKeyMap[AKEYCODE_U] = VUKEY_U;
	mKeyMap[AKEYCODE_V] = VUKEY_V;
	mKeyMap[AKEYCODE_W] = VUKEY_W;
	mKeyMap[AKEYCODE_X] = VUKEY_X;
	mKeyMap[AKEYCODE_Y] = VUKEY_Y;
	mKeyMap[AKEYCODE_Z] = VUKEY_Z;

	mKeyMap[AKEYCODE_DEL]			= VUKEY_BACK;
	mKeyMap[AKEYCODE_ENTER]			= VUKEY_ENTER;
	mKeyMap[AKEYCODE_SPACE]			= VUKEY_SPACE;
	mKeyMap[AKEYCODE_TAB]			= VUKEY_TAB;
	mKeyMap[AKEYCODE_COMMA]			= VUKEY_COMMA;
	mKeyMap[AKEYCODE_PERIOD]		= VUKEY_PERIOD;
	mKeyMap[AKEYCODE_MINUS]			= VUKEY_MINUS;
	mKeyMap[AKEYCODE_PLUS]			= VUKEY_PLUS;
	mKeyMap[AKEYCODE_APOSTROPHE]	= VUKEY_APOSTROPHE;
	mKeyMap[AKEYCODE_SEMICOLON]		= VUKEY_SEMICOLON;
	mKeyMap[AKEYCODE_SLASH]			= VUKEY_SLASH;
	mKeyMap[AKEYCODE_GRAVE]			= VUKEY_GRAVE;
	mKeyMap[AKEYCODE_BACKSLASH]		= VUKEY_BACKSLASH;
	mKeyMap[AKEYCODE_LEFT_BRACKET]	= VUKEY_LEFT_BRACKET;
	mKeyMap[AKEYCODE_RIGHT_BRACKET]	= VUKEY_RIGHT_BRACKET;

	mKeyMap[AKEYCODE_DPAD_LEFT]		= VUKEY_LEFT;
	mKeyMap[AKEYCODE_DPAD_RIGHT]	= VUKEY_RIGHT;
	mKeyMap[AKEYCODE_DPAD_UP]		= VUKEY_UP;
	mKeyMap[AKEYCODE_DPAD_DOWN]		= VUKEY_DOWN;
	mKeyMap[AKEYCODE_DPAD_CENTER]	= VUKEY_CENTER;

	mKeyMap[AKEYCODE_SHIFT_LEFT]	= VUKEY_SHIFT;
	mKeyMap[AKEYCODE_SHIFT_RIGHT]	= VUKEY_SHIFT;

	mKeyMap[AKEYCODE_ALT_LEFT]		= VUKEY_ALT;
	mKeyMap[AKEYCODE_ALT_RIGHT]		= VUKEY_ALT;

	return true;
}

//*****************************************************************************
void VuAndroidKeyboard::onKeyDown(VUUINT32 code)
{
	if ( code < 256 )
	{
		VUUINT32 key = mKeyMap[code];
		if ( key )
		{
			onKeyDownInternal(key);
		}
	}
}

//*****************************************************************************
void VuAndroidKeyboard::onKeyUp(VUUINT32 code)
{
	if ( code < 256 )
	{
		VUUINT32 key = mKeyMap[code];
		if ( key )
		{
			onKeyUpInternal(key);
		}
	}
}

#endif // VU_DISABLE_KEYBOARD
