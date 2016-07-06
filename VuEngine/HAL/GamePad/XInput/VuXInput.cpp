//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to GamePad library.
//
//  Uses XInput
// 
//*****************************************************************************

#include "VuXInput.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Events/VuEventManager.h"
#include "VuEngine/Math/VuMath.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Dev/VuDevConfig.h"


#define RECONNECT_INTERVAL_SECONDS (1.0f)
#define LOW_FREQ_VIBRATION_DECAY_RATE (4.0f) // per second
#define HIGH_FREQ_VIBRATION_DECAY_RATE (1.0f) // per second

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
VuXInput::VuXInput()
: mReconnectTimer(RECONNECT_INTERVAL_SECONDS)
, mbRumbleActive(true)
{
	addAxis("LEFT_STICK_X", -1.0f, 1.0f);
	addAxis("LEFT_STICK_Y", -1.0f, 1.0f);
	addAxis("RIGHT_STICK_X", -1.0f, 1.0f);
	addAxis("RIGHT_STICK_Y", -1.0f, 1.0f);
	addAxis("LEFT_TRIGGER", 0.0f, 1.0f);
	addAxis("RIGHT_TRIGGER", 0.0f, 1.0f);

	VUASSERT(mAxisDefs.size() == AXIS_COUNT, "VuXInput::VuXInput() axis count mismatch");

	addButton("DPAD_UP");
	addButton("DPAD_DOWN");
	addButton("DPAD_LEFT");
	addButton("DPAD_RIGHT");
	addButton("START");
	addButton("BACK");
	addButton("LEFT_THUMB");
	addButton("RIGHT_THUMB");
	addButton("LEFT_SHOULDER");
	addButton("RIGHT_SHOULDER");
	addButton("???");
	addButton("BIGBUTTON");
	addButton("A");
	addButton("B");
	addButton("X");
	addButton("Y");

	mpControllers = new VuXInputController[MAX_NUM_PADS];
}

//*****************************************************************************
VuXInput::~VuXInput()
{
	delete[] mpControllers;
}

//*****************************************************************************
bool VuXInput::init()
{
	if ( !VuGamePad::init() )
		return false;

	// find connected pads
	detectNewlyConnectedPads(false);
	
	// register phased tick
	VuTickManager::IF()->registerHandler(this, &VuXInput::tick, "Input");

	return true;
}

//*****************************************************************************
void VuXInput::postInit()
{
	if ( VuDevConfig::IF() )
	{
		VuDevConfig::IF()->getParam("RumbleActive").getValue(mbRumbleActive);
	}
}

//*****************************************************************************
void VuXInput::release()
{
	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
void VuXInput::tick(float fdt)
{
	mReconnectTimer -= VuTickManager::IF()->getRealDeltaTime();

	if(mReconnectTimer <= 0.0f)
	{
		detectNewlyConnectedPads(true);

		mReconnectTimer = RECONNECT_INTERVAL_SECONDS;
	}

	for ( int i = 0; i < MAX_NUM_PADS; i++ )
	{
		VuXInputController &controller = mpControllers[i];

		if ( controller.mIsConnected )
		{
			XINPUT_STATE state;
			DWORD result = XInputGetState(i, &state);

			if(result == ERROR_SUCCESS)
			{
				controller.mButtons = state.Gamepad.wButtons;
				controller.mAxes[AXIS_LEFT_STICK_X] = state.Gamepad.sThumbLX/32768.0f;
				controller.mAxes[AXIS_LEFT_STICK_Y] = state.Gamepad.sThumbLY/32768.0f;
				controller.mAxes[AXIS_RIGHT_STICK_X] = state.Gamepad.sThumbRX/32768.0f;
				controller.mAxes[AXIS_RIGHT_STICK_Y] = state.Gamepad.sThumbRY/32768.0f;
				controller.mAxes[AXIS_LEFT_TRIGGER] = state.Gamepad.bLeftTrigger/255.0f;
				controller.mAxes[AXIS_RIGHT_TRIGGER] = state.Gamepad.bRightTrigger/255.0f;

				controller.conditionByDeviceType();
			}
			else if(result == ERROR_DEVICE_NOT_CONNECTED)
			{
				VUPRINTF("Pad %d disconnected\n", i);
				controller.mIsConnected = false;

				controller.zero();

				VuEventManager::IF()->broadcast("OnPauseActionGame");

				VuEventManager::IF()->broadcastDelayed(0.25f, true, "OnGamePadDisconnected");
			}
			else
			{
				VUPRINTF("Pad %d returned code 0x%X\n", i, result);
			}
		}
	}

	// update vibration
	if ( mbRumbleActive )
	{
		for ( int i = 0; i < MAX_NUM_PADS; i++ )
		{
			VuXInputController &controller = mpControllers[i];

			if ( controller.mIsConnected )
			{
				XINPUT_VIBRATION vibration;
				if ( fdt == 0 )
				{
					vibration.wLeftMotorSpeed = 0;
					vibration.wRightMotorSpeed = 0;
				}
				else
				{							
					vibration.wLeftMotorSpeed = VuRound(65535*controller.mLowFreq);
					vibration.wRightMotorSpeed = VuRound(65535*controller.mHighFreq);
				}
				XInputSetState(i, &vibration);
			}

			controller.mLowFreq -= LOW_FREQ_VIBRATION_DECAY_RATE*fdt;
			controller.mHighFreq -= HIGH_FREQ_VIBRATION_DECAY_RATE*fdt;
			controller.mLowFreq = VuClamp(controller.mLowFreq, 0.0f, 1.0f);
			controller.mHighFreq = VuClamp(controller.mHighFreq, 0.0f, 1.0f);
		}
	}
}

//*****************************************************************************
void VuXInput::detectNewlyConnectedPads(bool sendMsg)
{
	for(int i = 0; i < MAX_NUM_PADS; i++)
	{
		VuXInputController &controller = mpControllers[i];

		// Check to see if any of the disconnected pads are connected.
		if ( !controller.mIsConnected )
		{
			XINPUT_STATE state;
			if ( XInputGetState(i, &state) == ERROR_SUCCESS )
			{
				if ( XInputGetCapabilities(i, 0, &controller.mCaps) == ERROR_SUCCESS )
				{
					VUPRINTF("Device %d connected\n", i);
					controller.mIsConnected = true;
					controller.mDeviceType = identifyDeviceType(controller.mCaps);

					if ( sendMsg )
					{
						VuParams params;
						params.addInt(i);
						VuEventManager::IF()->broadcast("OnGamePadConnected", params);
					}
				}
			}
		}
	}
}

//*****************************************************************************
VuXInput::VuXInputController::VuXInputController():
	mLowFreq(0.0f),
	mHighFreq(0.0f)
{
	memset(&mCaps, 0, sizeof(mCaps));
}

//*****************************************************************************
void VuXInput::VuXInputController::playVibrationEffect(int effect)
{
	mLowFreq = VuMax(mLowFreq, sVibrationEffects[effect].mLowFreq);
	mHighFreq = VuMax(mHighFreq, sVibrationEffects[effect].mHighFreq);
}

//*****************************************************************************
void VuXInput::VuXInputController::conditionByDeviceType()
{
	if ( mDeviceType == DEVICE_GAMEPAD )
	{
		float leftStickInnerDeadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE/32768.0f;
		float rightStickInnerDeadzone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE/32768.0f;

		// handle inner deadzone
		#define HANDLE_INNER_DEADZONE(value, threshold)			\
		{														\
			if ( value < -threshold )							\
				value = (value + threshold)/(1 - threshold);	\
			else if ( value > threshold )						\
				value = (value - threshold)/(1 - threshold);	\
			else												\
				value = 0;										\
		}

		HANDLE_INNER_DEADZONE(mAxes[AXIS_LEFT_STICK_X], leftStickInnerDeadzone);
		HANDLE_INNER_DEADZONE(mAxes[AXIS_LEFT_STICK_Y], leftStickInnerDeadzone);
		HANDLE_INNER_DEADZONE(mAxes[AXIS_RIGHT_STICK_X], rightStickInnerDeadzone);
		HANDLE_INNER_DEADZONE(mAxes[AXIS_RIGHT_STICK_Y], rightStickInnerDeadzone);

		#undef HANDLE_INNER_DEADZONE

		// handle outer deadzone
		float outerDeadzone = 3500/32768.0f;
		float outerFactor = 1.0f/(1.0f - outerDeadzone);

		#define HANDLE_OUTER_DEADZONE(value) value = VuClamp(value*outerFactor, -1.0f, 1.0f);

		HANDLE_OUTER_DEADZONE(mAxes[AXIS_LEFT_STICK_X]);
		HANDLE_OUTER_DEADZONE(mAxes[AXIS_LEFT_STICK_Y]);
		HANDLE_OUTER_DEADZONE(mAxes[AXIS_RIGHT_STICK_X]);
		HANDLE_OUTER_DEADZONE(mAxes[AXIS_RIGHT_STICK_Y]);

		#undef HANDLE_OUTER_DEADZONE

		// massage input
		mAxes[AXIS_LEFT_STICK_X] *= VuAbs(mAxes[AXIS_LEFT_STICK_X]);
		mAxes[AXIS_LEFT_STICK_Y] *= VuAbs(mAxes[AXIS_LEFT_STICK_Y]);
		mAxes[AXIS_RIGHT_STICK_X] *= VuAbs(mAxes[AXIS_RIGHT_STICK_X]);
		mAxes[AXIS_RIGHT_STICK_Y] *= VuAbs(mAxes[AXIS_RIGHT_STICK_Y]);
	}
}
