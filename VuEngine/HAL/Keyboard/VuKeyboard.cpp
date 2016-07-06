//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class for Keyboard.
//
//*****************************************************************************

#include "VuKeyboard.h"
#include "VuEngine/Math/VuMath.h"
#include "VuEngine/Managers/VuTickManager.h"


#if !VU_DISABLE_KEYBOARD

static const char *sKeyNames[] =
{
	"NONE", // VUKEY_NONE = 0,

	"0", // VUKEY_0,
	"1", // VUKEY_1,
	"2", // VUKEY_2,
	"3", // VUKEY_3,
	"4", // VUKEY_4,
	"5", // VUKEY_5,
	"6", // VUKEY_6,
	"7", // VUKEY_7,
	"8", // VUKEY_8,
	"9", // VUKEY_9,

	"A", // VUKEY_A,
	"B", // VUKEY_B,
	"C", // VUKEY_C,
	"D", // VUKEY_D,
	"E", // VUKEY_E,
	"F", // VUKEY_F,
	"G", // VUKEY_G,
	"H", // VUKEY_H,
	"I", // VUKEY_I,
	"J", // VUKEY_J,
	"K", // VUKEY_K,
	"L", // VUKEY_L,
	"M", // VUKEY_M,
	"N", // VUKEY_N,
	"O", // VUKEY_O,
	"P", // VUKEY_P,
	"Q", // VUKEY_Q,
	"R", // VUKEY_R,
	"S", // VUKEY_S,
	"T", // VUKEY_T,
	"U", // VUKEY_U,
	"V", // VUKEY_V,
	"W", // VUKEY_W,
	"X", // VUKEY_X,
	"Y", // VUKEY_Y,
	"Z", // VUKEY_Z,

	"BACK", // VUKEY_BACK,
	"ENTER", // VUKEY_ENTER,
	"SPACE", // VUKEY_SPACE,
	"TAB", // VUKEY_TAB,
	"COMMA", // VUKEY_COMMA,
	"PERIOD", // VUKEY_PERIOD,
	"MINUS", // VUKEY_MINUS,
	"PLUS", // VUKEY_PLUS,
	"APOSTROPHE", // VUKEY_APOSTROPHE,
	"SEMICOLON", // VUKEY_SEMICOLON,
	"SLASH", // VUKEY_SLASH,
	"GRAVE", // VUKEY_GRAVE,
	"BACKSLASH", // VUKEY_BACKSLASH,
	"LEFT BRACKET", // VUKEY_LEFT_BRACKET,
	"RIGHT BRACKET", // VUKEY_RIGHT_BRACKET,

	"LEFT", // VUKEY_LEFT,
	"RIGHT", // VUKEY_RIGHT,
	"UP", // VUKEY_UP,
	"DOWN", // VUKEY_DOWN,
	"CENTER", // VUKEY_CENTER,

	"SHIFT", // VUKEY_SHIFT,
	"ALT", // VUKEY_ALT,

	"CONTROL", // VUKEY_CONTROL,
	"ESCAPE", // VUKEY_ESCAPE,
	"SCROLL", // VUKEY_SCROLL,
	"PAUSE", // VUKEY_PAUSE,
	"INSERT", // VUKEY_INSERT,
	"DELETE", // VUKEY_DELETE,
	"HOME", // VUKEY_HOME,
	"END", // VUKEY_END,
	"PRIOR", // VUKEY_PRIOR,
	"NEXT", // VUKEY_NEXT,
	"F1", // VUKEY_F1,
	"F2", // VUKEY_F2,
	"F3", // VUKEY_F3,
	"F4", // VUKEY_F4,
	"F5", // VUKEY_F5,
	"F6", // VUKEY_F6,
	"F7", // VUKEY_F7,
	"F8", // VUKEY_F8,
	"F9", // VUKEY_F9,
	"F10", // VUKEY_F10,
	"F11", // VUKEY_F11,
	"F12", // VUKEY_F12,
};

VU_COMPILE_TIME_ASSERT(sizeof(sKeyNames)/sizeof(sKeyNames[0]) == VUKEY_COUNT);


//*****************************************************************************
VuKeyboard::VuKeyboard():
	mFocusPriority(0)
{
	memset(mKeyIsDown, 0, sizeof(mKeyIsDown));
	memset(mKeyWasDown, 0, sizeof(mKeyWasDown));
	memset(mKeyWasDownPrev, 0, sizeof(mKeyWasDownPrev));
	memset(mKeyDebounced, 0, sizeof(mKeyDebounced));
}

//*****************************************************************************
bool VuKeyboard::init()
{
	// register phased tick
	VuTickManager::IF()->registerHandler(this, &VuKeyboard::tick, "Input");

	return true;
}

//*****************************************************************************
void VuKeyboard::release()
{
	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
void VuKeyboard::addCallback(Callback *pCB, bool devOnly)
{
#if VU_DISABLE_DEV_KEYBOARD
	if ( devOnly )
		return;
#endif

	mCallbacks.push_back(VuCallbackEntry(pCB));
}

//*****************************************************************************
void VuKeyboard::removeCallback(Callback *pCB)
{
	for ( Callbacks::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++ )
	{
		if ( iter->mpCallback == pCB )
		{
			mCallbacks.erase(iter);
			break;
		}
	}

	recalculateFocusPriority();
}

//*****************************************************************************
void VuKeyboard::setCallbackPriority(Callback *pCB, VUUINT32 priority)
{
	for ( Callbacks::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++ )
		if ( iter->mpCallback == pCB )
			iter->mPriority = priority;

	recalculateFocusPriority();
}

//*****************************************************************************
const char *VuKeyboard::getKeyName(VUUINT32 key)
{
	if ( key < VUKEY_COUNT )
		return sKeyNames[key];

	return "";
}

//*****************************************************************************
VUUINT32 VuKeyboard::getKeyIndex(const char *name)
{
	for ( int i = 0; i < VUKEY_COUNT; i++ )
		if ( strcmp(sKeyNames[i], name) == 0 )
			return i;

	return VUKEY_NONE;
}

//*****************************************************************************
void VuKeyboard::onKeyDownInternal(VUUINT32 key)
{
	mKeyIsDown[key] = true;
	mKeyWasDown[key] = true;

	for ( Callbacks::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++ )
		if ( iter->mPriority == mFocusPriority )
			iter->mpCallback->onKeyDown(key);
}

//*****************************************************************************
void VuKeyboard::onKeyUpInternal(VUUINT32 key)
{
	mKeyIsDown[key] = false;

	for ( Callbacks::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++ )
		if ( iter->mPriority == mFocusPriority )
			iter->mpCallback->onKeyUp(key);
}

//*****************************************************************************
void VuKeyboard::tick(float fdt)
{
	for ( int i = 0; i < VUKEY_COUNT; i++ )
	{
		mKeyDebounced[i] = mKeyWasDown[i] & (~mKeyWasDownPrev[i]);
		mKeyWasDownPrev[i] = mKeyWasDown[i];
		mKeyWasDown[i] = false;
	}
}

//*****************************************************************************
void VuKeyboard::recalculateFocusPriority()
{
	mFocusPriority = 0;
	for ( Callbacks::iterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++ )
		mFocusPriority = VuMax(mFocusPriority, iter->mPriority);
}

#endif // VU_DISABLE_KEYBOARD
