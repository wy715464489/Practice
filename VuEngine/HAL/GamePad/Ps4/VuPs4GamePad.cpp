//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ps4 interface class for GamePad.
//
//*****************************************************************************
#include <pad.h>

#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/Ps4/VuPs4ProfileManager.h"
#include "VuEngine/Events/VuEventManager.h"
#include "VuEngine/Method/VuParams.h"
#include "VuEngine/HAL/GamePad/Ps4/VuPs4GamePad.h"
#include "VuEngine/Dev/VuDevConfig.h"

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuGamePad, VuPs4GamePad);


// vibration effects
struct VibrationEffect { float mLowFreq; float mHighFreq; };
static VibrationEffect sVibrationEffects[] =
{
	{ 0.0f, 0.5f }, // EFFECT_COLLISION_SMALL,
	{ 0.0f, 0.8f }, // EFFECT_COLLISION_MEDIUM,
	{ 0.0f, 1.0f }, // EFFECT_COLLISION_LARGE,
	{ 0.4f, 0.0f }, // EFFECT_SPLASH_SMALL,
	{ 0.7f, 0.0f }, // EFFECT_SPLASH_MEDIUM,
	{ 1.0f, 0.0f }, // EFFECT_SPLASH_LARGE,
};
VU_COMPILE_TIME_ASSERT(sizeof(sVibrationEffects) / sizeof(sVibrationEffects[0]) == VuGamePad::VuController::NUM_EFFECTS);

#define LOW_FREQ_VIBRATION_DECAY_RATE (2.0f) // per second
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

//*****************************************************************************
VuPs4GamePad::VuPs4GamePad() 
	: mRumbleActive(true)
{
	addAxis("LEFT_STICK_X", -1.0f, 1.0f);
	addAxis("LEFT_STICK_Y", -1.0f, 1.0f);
	addAxis("RIGHT_STICK_X", -1.0f, 1.0f);
	addAxis("RIGHT_STICK_Y", -1.0f, 1.0f);

	addAxis("LEFT_TRIGGER", 0.0f, 1.0f);
	addAxis("RIGHT_TRIGGER", 0.0f, 1.0f);

	VUASSERT(mAxisDefs.size() == AXIS_COUNT, "VuPs4GamePad::VuPs4GamePad() axis count mismatch");

	addButton("LEFT_THUMB");	// 2
	addButton("RIGHT_THUMB");	// 4
	addButton("OPTIONS");		// 8
	addButton("DPAD_UP");		// 16
	addButton("DPAD_RIGHT");	// 32
	addButton("DPAD_DOWN");		// 64
	addButton("DPAD_LEFT");		// 128

	// digital versions of L/R throttles
	addButton("L2");			// 256
	addButton("R2");			// 512
	// "bumper" digital buttons
	addButton("L1");			// 1024	
	addButton("R1");			// 2048

	addButton("TRIANGLE");		// 4096
	addButton("CIRCLE");		// 8192
	addButton("CROSS");			// 16384
	addButton("SQUARE");		// 32768
}

//*****************************************************************************
static VUINT translatePs4Buttons(VUINT32 input)
{
	VUINT32 output = input >> 1;

	return output;
}

//*****************************************************************************
bool VuPs4GamePad::init()
{
	if (!VuGamePad::init())
	{
		return false;
	}

	// register phased tick
	VuTickManager::IF()->registerHandler(this, &VuPs4GamePad::tick, "Input");

	return true;
}

//*****************************************************************************
void VuPs4GamePad::postInit()
{
	if (VuDevConfig::IF())
	{
		VuDevConfig::IF()->getParam("RumbleActive").getValue(mRumbleActive);
	}

	inspectPs4Controllers();
}

//*****************************************************************************
void VuPs4GamePad::release()
{
	VuGamePad::release();

	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
VUINT VuPs4GamePad::findVuController(SceUserServiceUserId userId)
{
	for (VUUINT i = 0; i < MAX_NUM_PADS; i++)
	{
		VuPs4Controller &controller = mControllers[i];

		if (controller.mIsConnected && userId == controller.mUserId)
		{
			return i;
		}
	}

	return MAX_NUM_PADS;
}

//*****************************************************************************
void VuPs4GamePad::addVuController(SceUserServiceUserId userId, int handle, eDeviceType deviceType, float deadZoneLeft, float deadZoneRight)
{
	for (int i = 0; i < MAX_NUM_PADS; i++)
	{
		VuPs4Controller& controller = mControllers[i];

		if (controller.mIsConnected)
		{
			continue;
		}
		else
		{
			controller.zero();
			controller.mIsConnected = true;
			controller.mHandle = handle;
			controller.mUserId = userId;
			controller.mDeviceType = deviceType;
			controller.mDeadZoneLeft = deadZoneLeft;
			controller.mDeadZoneRight = deadZoneRight;

			VuParams params;
			params.addInt(i);
			VuEventManager::IF()->broadcast("OnGamePadConnected", params);

			VUPRINTF("STATUS: VuPs4Controller Added (Index=%d) for userId=%#.8x deviceType=%d\n", i, userId, deviceType);

			break;
		}
	}
}

//*****************************************************************************
void VuPs4GamePad::removeVuController(SceUserServiceUserId userId)
{
	VUINT index = findVuController(userId);
	if (index != MAX_NUM_PADS)
	{
		VuPs4Controller& controller = mControllers[index];
		controller.zero();
		controller.mIsConnected = false;
		controller.mUserId = SCE_USER_SERVICE_USER_ID_INVALID;
		controller.mHandle = -1;
		controller.mDeviceType = NUM_DEVICE_TYPES;

		VuEventManager::IF()->broadcast("OnPauseActionGame");

		VuEventManager::IF()->broadcastDelayed(0.25f, true, "OnGamePadDisconnected");

		VUPRINTF("STATUS: VuPs4Controller Removed (Index=%d) for userId=%#.8x\n", index, userId);
	}
}

//*****************************************************************************
void VuPs4GamePad::inspectPs4Controller(SceUserServiceUserId userId)
{
	VUASSERT(userId != SCE_USER_SERVICE_USER_ID_INVALID, "Invalid userId");

	int handle = 0;

	VUINT index = findVuController(userId);
	if (index != MAX_NUM_PADS)
	{
		// Not yet connected, so continue to poll
		handle = mControllers[index].mHandle;
	}
	else
	{
		handle = scePadOpen(userId, SCE_PAD_PORT_TYPE_STANDARD, 0, VUNULL);
		if (handle == SCE_PAD_ERROR_ALREADY_OPENED)
		{
			VUPRINTF("ERROR: VuPs4GamePad::inspectPs4Controllers(): scePadOpen() returned %0.8x\n", handle);
			return;
		}
		else if (handle < 0)
		{
			// Couldn't find a Standard controller, see if there's a Special controller like a wheel or something else
			//
			handle = scePadOpen(userId, SCE_PAD_PORT_TYPE_SPECIAL, 0, VUNULL);
			if (handle < 0)
			{
				// Can't find a pad for this user, skip them
				return;
			}
		}
	}


	ScePadData data;
	VUINT result = scePadReadState(handle, &data);
	if (result < SCE_OK)
	{
		// Skip user if we can't read the pad
		removeVuController(userId);

		return;
	}

	if (data.connected)
	{
		// If the device is connected, but the VuController is not,
		// create the VuController
		if (index == MAX_NUM_PADS || !mControllers[index].mIsConnected)
		{
			ScePadControllerInformation info;
			memset(&info, 0, sizeof(info));
			result = scePadGetControllerInformation(handle, &info);
			if (result < SCE_OK)
			{
				VUPRINTF("ERROR: VuPs4GamePad::inspectPs4Controllers(): scePadGetControllerInformation() returned %0.8x\n", result);
			}
			else
			{
				eDeviceType deviceType = DEVICE_UNKNOWN;

				if (info.deviceClass == SCE_PAD_DEVICE_CLASS_STANDARD)
				{
					deviceType = DEVICE_GAMEPAD;
				}
				else if (info.deviceClass == SCE_PAD_DEVICE_CLASS_STEERING_WHEEL)
				{
					ScePadDeviceClassExtendedInformation extInfo;
					result = scePadDeviceClassGetExtendedInformation(handle, &extInfo);
					if (result < SCE_OK)
					{
						VUPRINTF("ERROR: VuPs4GamePad::inspectPs4Controllers(): scePadDeviceClassGetExtendedInformation() returned %0.8x\n", result);
					}

					// Check Steering Wheel capabilities to see if we can use it
					deviceType = DEVICE_STEERING_WHEEL;
				}

				addVuController(userId, handle, deviceType, ((float)info.stickInfo.deadZoneLeft)/255.0f, ((float)info.stickInfo.deadZoneRight)/255.0f);
			}
		}
	}
	else
	{
		result = scePadClose(handle);
		if (result < SCE_OK)
		{
			// closed the port
		}

		removeVuController(userId);
	}
}

//*****************************************************************************
void VuPs4GamePad::inspectPs4Controllers()
{
	// Run through all the users found on the Xb1 and see if we need to create controllers for them
	//
	SceUserServiceLoginUserIdList userIdList;

	VUINT result = sceUserServiceGetLoginUserIdList(&userIdList);
	if (result < SCE_OK)
	{
		VUPRINTF("WARNING: VuPs4GamePad::inspectPs4Controllers(): sceUserServiceGetLoginUserIdList() returned %0.8x\n", result);

		return;
	}

	for (int i = 0; i < SCE_USER_SERVICE_MAX_LOGIN_USERS; i++)
	{
		SceUserServiceUserId userId = userIdList.userId[i];

		// Skip invalid users
		if (userId != SCE_USER_SERVICE_USER_ID_INVALID)
		{
			inspectPs4Controller(userId);
		}
	}
}

//*****************************************************************************
void VuPs4GamePad::tick(float fdt)
{
	// Add or remove pads as they come online
	inspectPs4Controllers();

	// Update Valid Pads
	for (VUUINT i = 0; i < MAX_NUM_PADS; i++)
	{
		VuPs4Controller &controller = mControllers[i];

		if (controller.mIsConnected)
		{
			ScePadData data;
			VUINT result = scePadReadState(controller.mHandle, &data);
			if (result < SCE_OK)
			{
				VUPRINTF("ERROR: VuPs4GamePad::tick(): scePadReadState() returned %0.8x\n", result);

				// Skip user if we can't read the pad
				continue;
			}

			// Ignore input if the system has intercepted input
			if (data.buttons & SCE_PAD_BUTTON_INTERCEPTED)
			{
				continue;
			}

			// DEVICE_PAD/STICK/ETC
			//
			controller.mButtons = translatePs4Buttons(data.buttons);

			VuVector2 leftStick =  {(float)data.leftStick.x, (float)data.leftStick.y};
			VuVector2 rightStick = {(float)data.rightStick.x, (float)data.rightStick.y};

			leftStick.mX = leftStick.mX / 128.0f - 1.0f;
			leftStick.mY = -(leftStick.mY / 128.0f) + 1.0f;

			rightStick.mX = rightStick.mX / 128.0f - 1.0f;
			rightStick.mY = -(rightStick.mY / 128.0f) + 1.0f;

			// Compensate for dead zone
			//
			if ((leftStick.mX > -controller.mDeadZoneLeft) &&  (leftStick.mX < controller.mDeadZoneLeft))   leftStick.mX = 0.0f;
			if ((leftStick.mY > -controller.mDeadZoneLeft) &&  (leftStick.mY < controller.mDeadZoneLeft))   leftStick.mY = 0.0f;
			if ((rightStick.mX > -controller.mDeadZoneRight) && (rightStick.mX < controller.mDeadZoneRight)) rightStick.mX = 0.0f;
			if ((rightStick.mY > -controller.mDeadZoneRight) && (rightStick.mY < controller.mDeadZoneRight)) rightStick.mY = 0.0f;

			controller.mAxes[AXIS_LEFT_STICK_X] = leftStick.mX;
			controller.mAxes[AXIS_LEFT_STICK_Y] = leftStick.mY;
			controller.mAxes[AXIS_RIGHT_STICK_X] = rightStick.mX;
			controller.mAxes[AXIS_RIGHT_STICK_Y] = rightStick.mY;

			controller.mAxes[AXIS_LEFT_TRIGGER] = ((float)data.analogButtons.l2) / 255.0f;
			controller.mAxes[AXIS_RIGHT_TRIGGER] = ((float)data.analogButtons.r2) / 255.0f;

			// If it's a wheel, override the default data with the wheel data
			if (controller.mDeviceType == DEVICE_STEERING_WHEEL)
			{
				ScePadDeviceClassData deviceClassData;
				memset(&deviceClassData, 0, sizeof(deviceClassData));

				VUINT result = scePadDeviceClassParseData(controller.mHandle, &data, &deviceClassData);
				if (result < SCE_OK)
				{
					// Error getting wheel data
				}
				else
				{
					VUUINT16 throttle = deviceClassData.classData.steeringWheel.acceleratorPedal;
					/*E Release            : 0x0000
						Full pressed       : 0xffff */
					controller.mAxes[AXIS_RIGHT_TRIGGER] = ((float)throttle) / 65535.0f;

					VUUINT16 brake = deviceClassData.classData.steeringWheel.brakePedal;
					/*E Release            : 0x0000
						Full pressed       : 0xffff */
					controller.mAxes[AXIS_LEFT_TRIGGER] = ((float)brake) / 65535.0f;

					VUUINT16 steering = deviceClassData.classData.steeringWheel.steeringWheel;
					/*E Full turn to left  : 0x0000
						Center             : 0x8000
						Full tunn to right : 0xffff */
					controller.mAxes[AXIS_LEFT_STICK_X] = (((float)steering) / (65535.0f/2.0f)) - 1.0f;
				}
			}


			// update vibration
			if (mRumbleActive)
			{
				ScePadVibrationParam vibrate;
				vibrate.smallMotor = VuFloorInt(controller.mHighFreq * 255.0);
				vibrate.largeMotor = VuFloorInt(controller.mLowFreq * 255.0);

				result = scePadSetVibration(controller.mHandle, &vibrate);
				if (result < SCE_OK)
				{
					VUPRINTF("ERROR: VuPs4GamePad::tick(): scePadReadState() returned %0.8x\n", result);
				}
			}

			float rdt = VuTickManager::IF()->getRealDeltaTime();

			controller.mLowFreq -= LOW_FREQ_VIBRATION_DECAY_RATE * rdt;
			controller.mHighFreq -= HIGH_FREQ_VIBRATION_DECAY_RATE * rdt;
			controller.mLowFreq = VuMax(controller.mLowFreq, 0.0f);
			controller.mHighFreq = VuMax(controller.mHighFreq, 0.0f);
		}
	}
}

//*****************************************************************************
void VuPs4GamePad::VuPs4Controller::playVibrationEffect(int effect)
{
	mLowFreq = VuMax(mLowFreq, sVibrationEffects[effect].mLowFreq);
	mHighFreq = VuMax(mHighFreq, sVibrationEffects[effect].mHighFreq);
}
