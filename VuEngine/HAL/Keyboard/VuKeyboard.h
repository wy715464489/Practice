//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class for Keyboard.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"

class VuEngine;

// keys

enum
{
	VUKEY_NONE = 0,

	VUKEY_0,
	VUKEY_1,
	VUKEY_2,
	VUKEY_3,
	VUKEY_4,
	VUKEY_5,
	VUKEY_6,
	VUKEY_7,
	VUKEY_8,
	VUKEY_9,

	VUKEY_A,
	VUKEY_B,
	VUKEY_C,
	VUKEY_D,
	VUKEY_E,
	VUKEY_F,
	VUKEY_G,
	VUKEY_H,
	VUKEY_I,
	VUKEY_J,
	VUKEY_K,
	VUKEY_L,
	VUKEY_M,
	VUKEY_N,
	VUKEY_O,
	VUKEY_P,
	VUKEY_Q,
	VUKEY_R,
	VUKEY_S,
	VUKEY_T,
	VUKEY_U,
	VUKEY_V,
	VUKEY_W,
	VUKEY_X,
	VUKEY_Y,
	VUKEY_Z,

	VUKEY_BACK,
	VUKEY_ENTER,
	VUKEY_SPACE,
	VUKEY_TAB,
	VUKEY_COMMA,
	VUKEY_PERIOD,
	VUKEY_MINUS,
	VUKEY_PLUS,
	VUKEY_APOSTROPHE,
	VUKEY_SEMICOLON,
	VUKEY_SLASH,
	VUKEY_GRAVE,
	VUKEY_BACKSLASH,
	VUKEY_LEFT_BRACKET,
	VUKEY_RIGHT_BRACKET,

	VUKEY_LEFT,
	VUKEY_RIGHT,
	VUKEY_UP,
	VUKEY_DOWN,
	VUKEY_CENTER,

	VUKEY_SHIFT,
	VUKEY_ALT,

	// non-standard
	VUKEY_CONTROL,
	VUKEY_ESCAPE,
	VUKEY_SCROLL,
	VUKEY_PAUSE,
	VUKEY_INSERT,
	VUKEY_DELETE,
	VUKEY_HOME,
	VUKEY_END,
	VUKEY_PRIOR,
	VUKEY_NEXT,
	VUKEY_F1,
	VUKEY_F2,
	VUKEY_F3,
	VUKEY_F4,
	VUKEY_F5,
	VUKEY_F6,
	VUKEY_F7,
	VUKEY_F8,
	VUKEY_F9,
	VUKEY_F10,
	VUKEY_F11,
	VUKEY_F12,

	VUKEY_COUNT
};


#if VU_DISABLE_KEYBOARD

	class VuKeyboard : VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuKeyboard)
	protected:
		friend class VuEngine;
		bool init() { return true; }
	public:
		class Callback
		{
		};
		void		addCallback(Callback *pCB, bool devOnly = true) {}
		void		removeCallback(Callback *pCB) {}
		void		setCallbackPriority(Callback *pCB, int priority) {}
		bool		isKeyDown(VUUINT32 key, VUUINT32 priority = 0xffffffff)	{ return false; }
		bool		wasKeyPressed(VUUINT32 key, VUUINT32 priority = 0xffffffff)	{ return false; }
		const char	*getKeyName(VUUINT32 key) { return ""; }
		VUUINT32	getKeyIndex(const char *name) { return 0; }
	};

#else

	class VuKeyboard : VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuKeyboard)

	protected:
		// called by engine
		friend class VuEngine;
		virtual bool init();
		virtual void release();

	public:
		VuKeyboard();

		// registration of callbacks
		class Callback
		{
		public:
			virtual void onKeyDown(VUUINT32 key) {}
			virtual void onKeyUp(VUUINT32 key) {}
		};
		void		addCallback(Callback *pCB, bool devOnly = true);
		void		removeCallback(Callback *pCB);
		void		setCallbackPriority(Callback *pCB, VUUINT32 priority);

		// current key state
		bool		isKeyDown(VUUINT32 key, VUUINT32 priority = 0xffffffff)	{ return priority >= mFocusPriority && mKeyIsDown[key]; }
		bool		wasKeyPressed(VUUINT32 key, VUUINT32 priority = 0xffffffff)	{ return priority >= mFocusPriority && mKeyDebounced[key]; }

		const char	*getKeyName(VUUINT32 key);
		VUUINT32	getKeyIndex(const char *name);

	protected:
		void			onKeyDownInternal(VUUINT32 key);
		void			onKeyUpInternal(VUUINT32 key);

		virtual void	tick(float fdt);

	private:
		class VuCallbackEntry
		{
		public:
			VuCallbackEntry(Callback *pCallback) : mpCallback(pCallback), mPriority(0) {}
			Callback	*mpCallback;
			VUUINT32	mPriority;
		};
		typedef std::list<VuCallbackEntry> Callbacks;

		void		recalculateFocusPriority();

		Callbacks	mCallbacks;
		VUUINT8		mKeyIsDown[VUKEY_COUNT];
		VUUINT8		mKeyWasDown[VUKEY_COUNT];
		VUUINT8		mKeyWasDownPrev[VUKEY_COUNT];
		VUUINT8		mKeyDebounced[VUKEY_COUNT];
		VUUINT32	mFocusPriority;
	};

#endif // VU_DISABLE_KEYBOARD
