//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Objective-C support for Ios Game Pad.
//
//*****************************************************************************

#pragma once


namespace VuIosGamePadObjC
{
	enum eAxes
	{
		AXIS_LEFT_STICK_X,
		AXIS_LEFT_STICK_Y,
		AXIS_RIGHT_STICK_X,
		AXIS_RIGHT_STICK_Y,
		AXIS_LEFT_TRIGGER,
		AXIS_RIGHT_TRIGGER,
		
		AXIS_COUNT
	};
	
	enum eButtons
	{
		BUTTON_A,
		BUTTON_B,
		BUTTON_X,
		BUTTON_Y,
		BUTTON_LEFT_SHOULDER,
		BUTTON_RIGHT_SHOULDER,
		BUTTON_DPAD_UP,
		BUTTON_DPAD_DOWN,
		BUTTON_DPAD_LEFT,
		BUTTON_DPAD_RIGHT,
		BUTTON_PAUSE,
		BUTTON_SIMPLE_A,
		BUTTON_SIMPLE_B,
		BUTTON_SIMPLE_X,
		BUTTON_SIMPLE_Y,
		BUTTON_SIMPLE_LEFT_SHOULDER,
		BUTTON_SIMPLE_RIGHT_SHOULDER,
		BUTTON_SIMPLE_DPAD_UP,
		BUTTON_SIMPLE_DPAD_DOWN,
		BUTTON_SIMPLE_DPAD_LEFT,
		BUTTON_SIMPLE_DPAD_RIGHT,

		BUTTON_COUNT
	};
	
	void update(int maxPadCount);
	
	bool getAvailableIndex(int &index);
	void setConnectedState(int index, bool isConnected);
	void setExtendedState(int index, float lsx, float lsy, float rsx, float rsy, float lt, float rt, unsigned int buttons);
	void setSimpleState(int index, unsigned int buttons);
	void onPauseHandlerFired(int index);
}
