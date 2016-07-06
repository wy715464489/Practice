//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to GamePad library.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/GamePad/VuGamePad.h"

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Xbox::Input;
using namespace Windows::Xbox::System;

#define LOW_FREQ_VIBRATION_DECAY_RATE (4.0f) // per second
#define HIGH_FREQ_VIBRATION_DECAY_RATE (1.0f) // per second

class VuXb1Controller;

class VuXb1GamePad : public VuGamePad
{
public:
	VuXb1GamePad();

	virtual bool	init();
	virtual void	postInit();
	virtual void	release();

	virtual VuController	&getController(int index) { return mControllers[index]; }

	IGamepad^		getPrimaryGamepad() { return mpPrimaryGamepad; }
	void			setPrimaryGamepad(IGamepad^ pad) { mpPrimaryGamepad = pad; }

	IGamepad^		getLastPressedGamepad() { return mLastPressedGamepad; }
	void			setLastPressedGamepad(IGamepad^ pad) { mLastPressedGamepad = pad; }
	bool			isAButtonPressed(IGamepad^ pad);

	static VuXb1GamePad *IF() { return static_cast<VuXb1GamePad *>(VuGamePad::IF()); }

	void			inspectXb1Controllers();

private:
	void			tick(float fdt);

	void			setupXb1EventHandlers();
	void			removeXb1EventHandlers();


	void			addGamepadAsController(IGamepad^ gamepad);
	void			removeGamepadAsController(IGamepad^ gamepad);

    void			xb1GamepadAdded(GamepadAddedEventArgs^ eventArgs);
    void			xb1GamepadRemoved(GamepadRemovedEventArgs^ eventArgs);
	void			xb1ControllerPairingChanged(ControllerPairingChangedEventArgs^ eventArgs);

	class VuXb1Controller : public VuController
	{
	public:
		VuXb1Controller() : mDeviceId(0), mLowFreq(0.0f), mHighFreq(0.0f) {}

		virtual void	playVibrationEffect(int effect);

		VUUINT64		mDeviceId;
		float			mLowFreq;
		float			mHighFreq;
	};

	void				clearXb1Controller(VuXb1Controller& controller);


	IGamepad^			findXboxPad(const VuXb1Controller& controller);
	VUINT				findVuController(IGamepad^ xboxPad);

	VuXb1Controller		mControllers[MAX_NUM_PADS];

	IGamepad^			mpPrimaryGamepad;
	IGamepad^			mLastPressedGamepad;

	// Vibration
	bool				mRumbleActive;

	// Tokens for tracking registered event handlers
    Windows::Foundation::EventRegistrationToken mGamepadAddedToken;
    Windows::Foundation::EventRegistrationToken mGamepadRemovedToken;
	Windows::Foundation::EventRegistrationToken mControllerPairingChangedToken;
};

