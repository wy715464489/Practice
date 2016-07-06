//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Xb1 interface class for GamePad.
//
//*****************************************************************************

#include "VuEngine/HAL/GamePad/VuGamePad.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Events/VuEventManager.h"
#include "VuEngine/Dev/VuDevConfig.h"

#include "VuEngine/HAL/GamePad/Xb1/VuXb1Gamepad.h"

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuGamePad, VuXb1GamePad);

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


// vibration ffects
struct VibrationEffect { float mLowFreq; float mHighFreq; };
static VibrationEffect sVibrationEffects[] =
{
	{ 0.0f, 0.4f }, // EFFECT_COLLISION_SMALL,
	{ 0.0f, 0.7f }, // EFFECT_COLLISION_MEDIUM,
	{ 0.0f, 1.0f }, // EFFECT_COLLISION_LARGE,
	{ 0.4f, 0.0f }, // EFFECT_SPLASH_SMALL,
	{ 0.7f, 0.0f }, // EFFECT_SPLASH_MEDIUM,
	{ 1.0f, 0.0f }, // EFFECT_SPLASH_LARGE,
};
VU_COMPILE_TIME_ASSERT(sizeof(sVibrationEffects)/sizeof(sVibrationEffects[0]) == VuGamePad::VuController::NUM_EFFECTS);


//*****************************************************************************
VuXb1GamePad::VuXb1GamePad():
	mRumbleActive(true)
{
	addAxis("LEFT_STICK_X", -1.0f, 1.0f);
	addAxis("LEFT_STICK_Y", -1.0f, 1.0f);
	addAxis("RIGHT_STICK_X", -1.0f, 1.0f);
	addAxis("RIGHT_STICK_Y", -1.0f, 1.0f);
	addAxis("LEFT_TRIGGER", 0.0f, 1.0f);
	addAxis("RIGHT_TRIGGER", 0.0f, 1.0f);

	VUASSERT(mAxisDefs.size() == AXIS_COUNT, "VuXInput::VuXInput() axis count mismatch");

	addButton("MENU"); // 4
	addButton("VIEW"); // 8
	addButton("A"); // 16
	addButton("B"); // 32
	addButton("X"); // 64
	addButton("Y"); // 128
	addButton("DPAD_UP"); // 256
	addButton("DPAD_DOWN"); // 512
	addButton("DPAD_LEFT"); // 1024
	addButton("DPAD_RIGHT"); // 2048
	addButton("LEFT_SHOULDER"); // 4096
	addButton("RIGHT_SHOULDER"); // 8192
	addButton("LEFT_THUMB"); // 16384
	addButton("RIGHT_THUMB"); // 32768
}

//*****************************************************************************
bool VuXb1GamePad::init()
{
	if ( !VuGamePad::init() )
		return false;

	// register phased tick
	VuTickManager::IF()->registerHandler(this, &VuXb1GamePad::tick, "Input");

	setupXb1EventHandlers();

	inspectXb1Controllers();

	return true;
}

//*****************************************************************************
void VuXb1GamePad::postInit()
{
	if ( VuDevConfig::IF() )
	{
		VuDevConfig::IF()->getParam("RumbleActive").getValue(mRumbleActive);
	}
}

//*****************************************************************************
void VuXb1GamePad::release()
{
	removeXb1EventHandlers();

	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);
}



//*****************************************************************************
// Called at startup to inspect paired gamepads
//
void VuXb1GamePad::inspectXb1Controllers()
{
	// Clear array
	for ( VUUINT i = 0; i < MAX_NUM_PADS; i++ )
	{
		VuXb1Controller &controller = mControllers[i];
		clearXb1Controller(controller);
	}

	// Look at list of pads and add them as controllers
	for (VUINT i=0; i<(int)Gamepad::Gamepads->Size; i++)
	{
		IGamepad^ pad = Gamepad::Gamepads->GetAt(i);

		// Only add a valid pad that's not already in the list
		if (pad != nullptr && findVuController(pad) == MAX_NUM_PADS)
		{
			addGamepadAsController(pad);
			
			// IF we haven't identified a primary gamepad yet, use
			// the first gamepad with a signed-in user
			if (getPrimaryGamepad() == nullptr && pad->User != nullptr)
			{
				// This is the first one in the list that has a user attached
				if (pad->User->IsSignedIn)
				{
					setPrimaryGamepad(pad);
				}
			}
		}
	}
}

//*****************************************************************************
void VuXb1GamePad::setupXb1EventHandlers()
{
	// Register a callback to our Sign-In Event Handler
	EventHandler<GamepadAddedEventArgs^>^ gamepadAddedEvent = ref new EventHandler<GamepadAddedEventArgs^>(
		[this] (Platform::Object^, GamepadAddedEventArgs^ eventArgs)
	{
		xb1GamepadAdded(eventArgs);
	});
	mGamepadAddedToken = Gamepad::GamepadAdded += gamepadAddedEvent;

	// Register a callback to our Sign-Out Event Handler
	EventHandler<GamepadRemovedEventArgs^>^ gamepadRemovedEvent = ref new EventHandler<GamepadRemovedEventArgs^>(
		[this] (Platform::Object^, GamepadRemovedEventArgs^ eventArgs)
	{
		xb1GamepadRemoved(eventArgs);
	});
	mGamepadRemovedToken = Gamepad::GamepadRemoved += gamepadRemovedEvent;

	EventHandler<ControllerPairingChangedEventArgs^>^ controllerPairingChangedEvent = ref new EventHandler<ControllerPairingChangedEventArgs^>(
		[this] (Platform::Object^, ControllerPairingChangedEventArgs^ eventArgs)
	{
	});
	mControllerPairingChangedToken = Controller::ControllerPairingChanged += controllerPairingChangedEvent;
}

//*****************************************************************************
void VuXb1GamePad::removeXb1EventHandlers()
{
    try
    {
		// Remove our event handlers from the system via the tokens we stored
		//
        Gamepad::GamepadAdded -= mGamepadAddedToken;
        Gamepad::GamepadRemoved -= mGamepadRemovedToken;
		Controller::ControllerPairingChanged -= mControllerPairingChangedToken;
    }

    catch(Platform::Exception^ )
    {
		VUWPRINTF(L"Exception in VuXb1GamePad::removeXb1EventHandlers()\n");
    }
}

//*****************************************************************************
IGamepad^ VuXb1GamePad::findXboxPad(const VuXb1Controller& controller)
{
	for (VUUINT j=0; j<Gamepad::Gamepads->Size; j++)
	{
		auto pad = Gamepad::Gamepads->GetAt(j);
		if (pad->Id == controller.mDeviceId)
		{
			return pad;
		}
	}

	return nullptr;
}

//*****************************************************************************
VUINT VuXb1GamePad::findVuController(IGamepad^ xboxPad)
{
	for ( VUUINT i = 0; i < MAX_NUM_PADS; i++ )
	{
		VuXb1Controller &controller = mControllers[i];
		
		if (controller.mIsConnected && xboxPad->Id == controller.mDeviceId)
		{
			return i;
		}
	}

	return MAX_NUM_PADS;
}

//*****************************************************************************
void VuXb1GamePad::clearXb1Controller(VuXb1Controller& controller)
{
	controller.zero();
	controller.mIsConnected = false;
	controller.mLowFreq = 0.0f;
	controller.mHighFreq = 0.0f;
	controller.mDeviceId = 0LL;
}

//*****************************************************************************
void VuXb1GamePad::tick(float fdt)
{
	bool pressedButton = false;

	for ( VUUINT i = 0; i < MAX_NUM_PADS; i++ )
	{
		VuXb1Controller &controller = mControllers[i];
		
		if (controller.mIsConnected)
		{
			auto pad = findXboxPad(controller);
			if (pad != nullptr)
			{
				if (!pressedButton && isAButtonPressed(pad))
				{
					setLastPressedGamepad(pad);
					pressedButton = true;
				}

				IGamepadReading^ reading = pad->GetCurrentReading();

				controller.mButtons = (VUUINT32)reading->Buttons >> 2; // Xb1 hardware buttons start at 4 (1<<2)

				controller.mAxes[AXIS_LEFT_STICK_X] = reading->LeftThumbstickX;
				controller.mAxes[AXIS_LEFT_STICK_Y] = reading->LeftThumbstickY;
				controller.mAxes[AXIS_RIGHT_STICK_X] = reading->RightThumbstickX;
				controller.mAxes[AXIS_RIGHT_STICK_Y] = reading->RightThumbstickY;
				controller.mAxes[AXIS_LEFT_TRIGGER] = reading->LeftTrigger;
				controller.mAxes[AXIS_RIGHT_TRIGGER] = reading->RightTrigger;

				// update vibration
				if ( mRumbleActive )
				{
					GamepadVibration vib;
					vib.LeftMotorLevel = controller.mLowFreq;
					vib.RightMotorLevel = controller.mHighFreq;
					vib.LeftTriggerLevel = 0.0f;
					vib.RightTriggerLevel = 0.0f;
					pad->SetVibration(vib);
				}

				controller.mLowFreq -= LOW_FREQ_VIBRATION_DECAY_RATE*fdt;
				controller.mHighFreq -= HIGH_FREQ_VIBRATION_DECAY_RATE*fdt;
				controller.mLowFreq = VuMax(controller.mLowFreq, 0.0f);
				controller.mHighFreq = VuMax(controller.mHighFreq, 0.0f);
			}
		}
	}
}

//*****************************************************************************
bool VuXb1GamePad::isAButtonPressed(IGamepad^ pad)
{
	IGamepadReading^ reading = pad->GetCurrentReading();

	if (reading->IsAPressed)
	{
		return true;
	}

	return false;
}

//*****************************************************************************
void VuXb1GamePad::addGamepadAsController(IGamepad^ gamepad)
{
	int firstFree = -1;

	for ( VUUINT i = 0; i < MAX_NUM_PADS; i++ )
	{
		VuXb1Controller &controller = mControllers[i];

		if (controller.mIsConnected)
		{
			continue;
		}
		else
		{
			if (firstFree == -1)
			{
				firstFree = i;
			}
		}
	}

	if (firstFree != -1)
	{
		auto &controller = mControllers[firstFree];
		clearXb1Controller(controller);

		controller.mIsConnected = true;
		controller.mDeviceId = gamepad->Id;

		VuParams params;
		params.addInt(firstFree);
		VuEventManager::IF()->broadcast("OnGamePadConnected", params);
	}
}

void VuXb1GamePad::removeGamepadAsController(IGamepad^ gamepad)
{
	VUINT index = findVuController(gamepad);
	if (index != MAX_NUM_PADS)
	{
		auto &controller = mControllers[index];
		clearXb1Controller(controller);

		VuEventManager::IF()->broadcast("OnPauseActionGame");

		VuEventManager::IF()->broadcastDelayed(0.25f, true, "OnGamePadDisconnected");
	}
}


//*****************************************************************************
void VuXb1GamePad::VuXb1Controller::playVibrationEffect(int effect)
{
	mLowFreq = VuMax(mLowFreq, sVibrationEffects[effect].mLowFreq);
	mHighFreq = VuMax(mHighFreq, sVibrationEffects[effect].mHighFreq);
}

//*****************************************************************************
void VuXb1GamePad::xb1GamepadAdded(GamepadAddedEventArgs^ eventArgs)
{
	// Manually add to mControllers
	auto gamepad = eventArgs->Gamepad;

	addGamepadAsController(gamepad);
}

//*****************************************************************************
void VuXb1GamePad::xb1GamepadRemoved(GamepadRemovedEventArgs^ eventArgs)
{
	// Manually null out from mControllers
	auto gamepad = eventArgs->Gamepad;

	removeGamepadAsController(gamepad);
}

//*****************************************************************************
void VuXb1GamePad::xb1ControllerPairingChanged(ControllerPairingChangedEventArgs^ eventArgs)
{
	auto controller = eventArgs->Controller;

	VUWPRINTF(L"CONTROLLER PAIRING CHANGED!\n");
}

