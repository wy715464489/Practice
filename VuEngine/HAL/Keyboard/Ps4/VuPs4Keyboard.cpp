//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class for Keyboard.
//
//*****************************************************************************

#include "VuPs4Keyboard.h"


#if VU_DISABLE_KEYBOARD

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuKeyboard, VuKeyboard);

#else
#include <dbg_keyboard.h>

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuKeyboard, VuPs4Keyboard);


//*****************************************************************************
VuPs4Keyboard::VuPs4Keyboard()
	:
	mKeyboardHandle(0),
	mCurList(0)
{
	memset(mKeyMap, 0, sizeof(mKeyMap));
}

//*****************************************************************************
bool VuPs4Keyboard::init()
{
	if ( !VuKeyboard::init() )
		return false;

	mKeyMap[SCE_DBG_KEYBOARD_CODE_0] = VUKEY_0;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_1] = VUKEY_1;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_2] = VUKEY_2;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_3] = VUKEY_3;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_4] = VUKEY_4;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_5] = VUKEY_5;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_6] = VUKEY_6;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_7] = VUKEY_7;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_8] = VUKEY_8;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_9] = VUKEY_9;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_A] = VUKEY_A;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_B] = VUKEY_B;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_C] = VUKEY_C;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_D] = VUKEY_D;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_E] = VUKEY_E;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_F] = VUKEY_F;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_G] = VUKEY_G;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_H] = VUKEY_H;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_I] = VUKEY_I;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_J] = VUKEY_J;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_K] = VUKEY_K;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_L] = VUKEY_L;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_M] = VUKEY_M;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_N] = VUKEY_N;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_O] = VUKEY_O;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_P] = VUKEY_P;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_Q] = VUKEY_Q;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_R] = VUKEY_R;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_S] = VUKEY_S;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_T] = VUKEY_T;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_U] = VUKEY_U;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_V] = VUKEY_V;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_W] = VUKEY_W;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_X] = VUKEY_X;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_Y] = VUKEY_Y;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_Z] = VUKEY_Z;

	mKeyMap[SCE_DBG_KEYBOARD_CODE_BS] = VUKEY_BACK;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_ENTER] = VUKEY_ENTER;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_SPACE] = VUKEY_SPACE;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_TAB] = VUKEY_TAB;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_COMMA] = VUKEY_COMMA;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_PERIOD] = VUKEY_PERIOD;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_MINUS] = VUKEY_MINUS;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_EQUAL_101] = VUKEY_PLUS;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_QUOTATION_101] = VUKEY_APOSTROPHE;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_SEMICOLON] = VUKEY_SEMICOLON;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_SLASH] = VUKEY_SLASH;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_ACCENT_CIRCONFLEX_106] = VUKEY_GRAVE;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_BACKSLASH_101] = VUKEY_BACKSLASH;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_LEFT_BRACKET_101] = VUKEY_LEFT_BRACKET;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_RIGHT_BRACKET_101] = VUKEY_RIGHT_BRACKET;

	mKeyMap[SCE_DBG_KEYBOARD_CODE_LEFT_ARROW] = VUKEY_LEFT;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_RIGHT_ARROW] = VUKEY_RIGHT;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_UP_ARROW] = VUKEY_UP;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_DOWN_ARROW] = VUKEY_DOWN;

	mKeyMap[SCE_DBG_KEYBOARD_CODE_ESCAPE] = VUKEY_ESCAPE;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_SCROLL_LOCK] = VUKEY_SCROLL;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_PAUSE] = VUKEY_PAUSE;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_INSERT] = VUKEY_INSERT;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_DELETE] = VUKEY_DELETE;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_HOME] = VUKEY_HOME;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_END] = VUKEY_END;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_PAGE_UP] = VUKEY_PRIOR;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_PAGE_DOWN] = VUKEY_NEXT;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_F1] = VUKEY_F1;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_F2] = VUKEY_F2;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_F3] = VUKEY_F3;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_F4] = VUKEY_F4;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_F5] = VUKEY_F5;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_F6] = VUKEY_F6;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_F7] = VUKEY_F7;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_F8] = VUKEY_F8;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_F9] = VUKEY_F9;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_F10] = VUKEY_F10;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_F11] = VUKEY_F11;
	mKeyMap[SCE_DBG_KEYBOARD_CODE_F12] = VUKEY_F12;

	SceUserServiceUserId userId;

	VUINT result = sceUserServiceGetInitialUser(&userId);
	if (result < SCE_OK)
	{
		VUPRINTF("ERROR: Unable to determine initial user, keyboard could not initialize.\n");

		return false;
	}

	mKeyboardHandle = sceDbgKeyboardOpen(userId, SCE_DBG_KEYBOARD_PORT_TYPE_STANDARD, 0, VUNULL);
	if (mKeyboardHandle < 0)
	{
		VUPRINTF("ERROR: Unable to open keyboard port, keyboard could not initialize.\n");
		return false;
	}

	mCurList = 0;
	mKeyList[0].clear();
	mKeyList[1].clear();

	return true;
}

//*****************************************************************************
void VuPs4Keyboard::release()
{
	VUINT result = sceDbgKeyboardClose(mKeyboardHandle);
	if (result < SCE_OK)
	{
		VUPRINTF("ERROR: Unable to Close the keyboard port.\n");
	}
}

//*****************************************************************************
void VuPs4Keyboard::tick(float fdt)
{
	SceDbgKeyboardData data;

	VUINT result = sceDbgKeyboardReadState(mKeyboardHandle, &data);
	if (result < SCE_OK)
	{
		return;
	}

	// data is valid
	if (!data.connected)
	{
		return;
	}

	// Switch key lists
	if (++mCurList > 1)
	{
		mCurList = 0;
	}

	mKeyList[mCurList].clear();

	std::vector<VUUINT32>& curList = mKeyList[mCurList];
	std::vector<VUUINT32>& prevList = mKeyList[(mCurList + 1) % 2];

	VUUINT32 modifiers = data.modifierKey;

	if ((modifiers & SCE_DBG_KEYBOARD_MKEY_L_CTRL) ||
		(modifiers & SCE_DBG_KEYBOARD_MKEY_R_CTRL))
	{
		curList.push_back(VUKEY_CONTROL);
	}

	if ((modifiers & SCE_DBG_KEYBOARD_MKEY_L_SHIFT) ||
		(modifiers & SCE_DBG_KEYBOARD_MKEY_R_SHIFT))
	{
		curList.push_back(VUKEY_SHIFT);
	}

	if ((modifiers & SCE_DBG_KEYBOARD_MKEY_L_ALT) ||
		(modifiers & SCE_DBG_KEYBOARD_MKEY_R_ALT))
	{
		curList.push_back(VUKEY_ALT);
	}

	if (data.length > 0)
	{
		// Special case, the 0-value "KEY OFF" code, which means something
		// was released or only a modifier is pressed
		if (data.length == 1)
		{
			if (data.keyCode[0] == 0)
			{
				if (curList.size() > 0)
				{
					// A modifier was pressed, and no other keys, so this isn't a 
					// key-up event
				}
				else
				{
					// No modifier was pressed, it's keys-up time now, lift
					// all that aren't currently down
					//
					// Use the key list from last tick, not the current one
					onKeysUp(prevList);
				}
			}
			else if (data.keyCode[0] == SCE_DBG_KEYBOARD_CODE_E_ROLLOVER)
			{
				// Rollover occured, too many keys have been pressed
				return;
			}
		}

		// Add all held keys that are valid
		for (VUINT i = 0; i < data.length; i++)
		{
			VUUINT32 key = data.keyCode[i];
			if (key != 0)
			{
				curList.push_back(mKeyMap[key]);
			}
		}

		// curList keys are down. For any not in the previous list, 
		// generate events
		onKeysDown(curList, prevList);
	}

	// If multiple keys have been held, see if there's anything in 
	// the previous list that's not in the current list and send KeyUp
	if (prevList.size() > 0)
	{
		std::vector<VUUINT32> releasedList;
		for (auto key : prevList)
		{
			auto it = std::find(curList.begin(), curList.end(), key);
			if (it == curList.end())
			{
				releasedList.push_back(key);
			}
		}
		onKeysUp(releasedList);
	}
}

//*****************************************************************************
void VuPs4Keyboard::onKeysDown(std::vector<VUUINT32>& curList, std::vector<VUUINT32>& prevList)
{
	for (auto key : curList)
	{
		auto it = std::find(prevList.begin(), prevList.end(), key);
		if (it == prevList.end())
		{
			onKeyDownInternal(key);

			//VUPRINTF("INFO: Key Down: %0.8x\n", key);
		}
	}
}

//*****************************************************************************
void VuPs4Keyboard::onKeysUp(std::vector<VUUINT32>& keyList)
{
	for (auto key : keyList)
	{
		onKeyUpInternal(key);

		//VUPRINTF("INFO: Key Up: %0.8x\n", key);
	}
}

#endif // VU_DISABLE_KEYBOARD
