//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android interface class for GamePad.
//
//*****************************************************************************

#include "VuAndroidGamePad.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Events/VuEventManager.h"
#include "VuEngine/Net/VuNearbyConnectionManager.h"
#include "VuEngine/Util/VuHash.h"

#define DISCONNECT_TIMER 1.0f // seconds


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuGamePad, VuAndroidGamePad);

// static JAVA stuff
static JNIEnv		*s_jniEnv;
static jobject		s_helperObject;
static jmethodID	s_isDeviceConnected;
static jmethodID	s_playVibration;


//*****************************************************************************
void VuAndroidGamePad::bindJavaMethods(JNIEnv *jniEnv, jobject classLoaderObject, jmethodID findClassMethod)
{
	__android_log_print(ANDROID_LOG_DEBUG, "GamePad",  "VuAndroidGamePad::bindJavaMethods()\n");

	s_jniEnv = jniEnv;

	// get reference to helper class object
	jstring helperClassName = jniEnv->NewStringUTF("com/vectorunit/VuGamePadHelper");
	jclass helperClass = (jclass)jniEnv->CallObjectMethod(classLoaderObject, findClassMethod, helperClassName);
	jniEnv->DeleteLocalRef(helperClassName);

	jmethodID getInstance = jniEnv->GetStaticMethodID(helperClass, "getInstance", "()Lcom/vectorunit/VuGamePadHelper;");
	jobject helperObject = jniEnv->CallStaticObjectMethod(helperClass, getInstance);
	s_helperObject = jniEnv->NewGlobalRef(helperObject);

	// methods
	s_isDeviceConnected = jniEnv->GetMethodID(helperClass, "isDeviceConnected", "(I)Z");
	s_playVibration = jniEnv->GetMethodID(helperClass, "playVibration", "(I)V");
}

//*****************************************************************************
VuAndroidGamePad::VuAndroidGamePad():
	mDisconnectTimer(0.0f)
{
	addAxis("X", -1.0f, 1.0f);
	addAxis("Y", -1.0f, 1.0f);
	addAxis("Z", -1.0f, 1.0f);
	addAxis("RX", -1.0f, 1.0f);
	addAxis("RY", -1.0f, 1.0f);
	addAxis("RZ", -1.0f, 1.0f);
	addAxis("LEFT_TRIGGER", 0.0f, 1.0f);
	addAxis("RIGHT_TRIGGER", 0.0f, 1.0f);
	addAxis("GAS", 0.0f, 1.0f);
	addAxis("BRAKE", 0.0f, 1.0f);
	addAxis("HAT_X", -1.0f, 1.0f);
	addAxis("HAT_Y", -1.0f, 1.0f);
	addAxis("MC", -1.0f, 1.0f);

	VUASSERT(getAxisCount() == AXIS_COUNT, "VuAndroidGamePad::VuAndroidGamePad() axis count mismatch");

	addButton("A");
	addButton("B");
	addButton("C");
	addButton("X");
	addButton("Y");
	addButton("Z");
	addButton("L1");
	addButton("R1");
	addButton("L2");
	addButton("R2");
	addButton("THUMBL");
	addButton("THUMBR");
	addButton("START");
	addButton("SELECT");
	addButton("MODE");
	addButton("DPAD_UP");
	addButton("DPAD_DOWN");
	addButton("DPAD_LEFT");
	addButton("DPAD_RIGHT");
	addButton("DPAD_CENTER");
	addButton("BACK");

	VUASSERT(getButtonCount() == BUTTON_COUNT, "VuAndroidGamePad::VuAndroidGamePad() button count mismatch");

	mpControllers = new VuAndroidController[MAX_NUM_PADS];
}

//*****************************************************************************
VuAndroidGamePad::~VuAndroidGamePad()
{
	delete[] mpControllers;
}

//*****************************************************************************
bool VuAndroidGamePad::init()
{
	if ( !VuGamePad::init() )
		return false;

	VuTickManager::IF()->registerHandler(this, &VuAndroidGamePad::tick, "Input");

	return true;
}

//*****************************************************************************
void VuAndroidGamePad::postInit()
{
	VuGamePad::postInit();

	if ( VuNearbyConnectionManager::IF() )
	{
		VuNearbyConnectionManager::IF()->addListener(this);

		if ( !VuSys::IF()->hasTouch() )
			VuNearbyConnectionManager::IF()->startAdvertising();
	}
}

//*****************************************************************************
void VuAndroidGamePad::preRelease()
{
	VuGamePad::preRelease();

	if ( VuNearbyConnectionManager::IF() )
	{
		VuNearbyConnectionManager::IF()->removeListener(this);

		if ( !VuSys::IF()->hasTouch() )
			VuNearbyConnectionManager::IF()->reset();
	}
}

//*****************************************************************************
void VuAndroidGamePad::release()
{
	VuTickManager::IF()->unregisterHandlers(this);

	VuGamePad::release();
}

//*****************************************************************************
void VuAndroidGamePad::onAxisEvent(int deviceId, eDeviceType deviceType, float axisX, float axisY, float axisZ, float axisRX, float axisRY, float axisRZ, float axisLTrigger, float axisRTrigger, float axisGas, float axisBrake, float axisHatX, float axisHatY)
{
	int padIndex = getPadIndex(deviceId);
	if ( padIndex >= 0 )
	{
		VuAndroidController &controller = mpControllers[padIndex];

		controller.mAxes[AXIS_X] = axisX;
		controller.mAxes[AXIS_Y] = axisY;
		controller.mAxes[AXIS_Z] = axisZ;
		controller.mAxes[AXIS_RX] = axisRX;
		controller.mAxes[AXIS_RY] = axisRY;
		controller.mAxes[AXIS_RZ] = axisRZ;
		controller.mAxes[AXIS_LEFT_TRIGGER] = axisLTrigger;
		controller.mAxes[AXIS_RIGHT_TRIGGER] = axisRTrigger;
		controller.mAxes[AXIS_GAS] = axisGas;
		controller.mAxes[AXIS_BRAKE] = axisBrake;
		controller.mAxes[AXIS_HAT_X] = axisHatX;
		controller.mAxes[AXIS_HAT_Y] = axisHatY;
		controller.mAxes[AXIS_MC] = 0.0f;

		controller.mDeviceType = deviceType;
	}
}

//*****************************************************************************
void VuAndroidGamePad::onButtonEvent(int deviceId, eDeviceType deviceType, int button, bool down)
{
	int padIndex = getPadIndex(deviceId);
	if ( padIndex >= 0 )
	{
		VuAndroidController &controller = mpControllers[padIndex];

		if ( down )
			controller.mButtons |= 1 << button;
		else
			controller.mButtons &= ~(1 << button);

		controller.mDeviceType = deviceType;
	}
}

//*****************************************************************************
void VuAndroidGamePad::tick(float fdt)
{
	float fdtReal = VuTickManager::IF()->getRealDeltaTime();

	mDisconnectTimer += fdtReal;
	if ( mDisconnectTimer >= DISCONNECT_TIMER )
	{
		mDisconnectTimer = 0.0f;

		// check for device disconnections
		bool broadcastDisconnectedEvent = false;
		for ( int i = 0; i < MAX_NUM_PADS; i++ )
		{
			VuAndroidController &controller = mpControllers[i];

			if ( controller.mIsConnected )
			{
				if (  controller.mDeviceType == DEVICE_MOBILE_CONTROLLER )
				{
					const VuNearbyConnectionManager::Endpoints &endpoints = VuNearbyConnectionManager::IF()->getConnectedEndpoints();
					if ( endpoints.find(controller.mEndpointId) == endpoints.end() )
					{
						controller.mIsConnected = false;
						controller.mDeviceId = -1;
						broadcastDisconnectedEvent = true;
					}
				}
				else
				{
					if ( !s_jniEnv->CallBooleanMethod(s_helperObject, s_isDeviceConnected, controller.mDeviceId) )
					{
						controller.mIsConnected = false;
						controller.mDeviceId = -1;
						broadcastDisconnectedEvent = true;
					}
				}
			}
		}

		// we do this on Android TV, 
		// we don't do this anymore on Android (some BT-LTE devices time-out in 30 secs)
		if (broadcastDisconnectedEvent && !VuSys::IF()->hasTouch())
		{
			VuEventManager::IF()->broadcast("OnPauseActionGame");

			VuEventManager::IF()->broadcastDelayed(0.25f, true, "OnGamePadDisconnected");
		}
	}
}

//*****************************************************************************
int VuAndroidGamePad::getPadIndex(int deviceId)
{
	for ( int i = 0; i < MAX_NUM_PADS; i++ )
		if ( mpControllers[i].mDeviceId == deviceId )
			return i;

	for ( int i = 0; i < MAX_NUM_PADS; i++ )
	{
		if ( !mpControllers[i].mIsConnected )
		{
			mpControllers[i].mIsConnected = true;
			mpControllers[i].mDeviceId = deviceId;
			return i;
		}
	}

	return -1;
}

//*****************************************************************************
void VuAndroidGamePad::onNCMessageReceived(const char *endpointId, const void *pData, int dataSize)
{
	if ( VuNCMobileControllerStateMsg::validate(pData, dataSize) )
	{
		VuNCMobileControllerStateMsg msg;
		memcpy(&msg, pData, sizeof(msg));

		VUUINT32 deviceId = VuHash::fnv32String(endpointId);
		int padIndex = getPadIndex(deviceId);
		if ( padIndex >= 0 )
		{
			VuAndroidController &controller = mpControllers[padIndex];

			controller.mButtons = msg.mButtons;
			controller.mAxes[AXIS_MC] = msg.mDeviceTilt;

			controller.mDeviceType = DEVICE_MOBILE_CONTROLLER;
			controller.mEndpointId = endpointId;
		}
	}
	else if ( VuNCPlayVibrationMsg::validate(pData, dataSize) )
	{
		VuNCPlayVibrationMsg msg;
		memcpy(&msg, pData, sizeof(msg));

		s_jniEnv->CallVoidMethod(s_helperObject, s_playVibration, msg.mEffect);
	}
}
