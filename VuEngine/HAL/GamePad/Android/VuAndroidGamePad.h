//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android interface class for GamePad.
//
//*****************************************************************************

#pragma once

#include <jni.h>
#include "VuEngine/HAL/GamePad/VuGamePad.h"
#include "VuEngine/Net/VuNearbyConnectionManager.h"


class VuAndroidGamePad : public VuGamePad, VuNearbyConnectionManager::Listener
{
public:
	enum eAxes
	{
		AXIS_X,
		AXIS_Y,
		AXIS_Z,
		AXIS_RX,
		AXIS_RY,
		AXIS_RZ,
		AXIS_LEFT_TRIGGER,
		AXIS_RIGHT_TRIGGER,
		AXIS_GAS,
		AXIS_BRAKE,
		AXIS_HAT_X,
		AXIS_HAT_Y,
		AXIS_MC,

		AXIS_COUNT
	};

	enum eButtons
	{
		BUTTON_A,
		BUTTON_B,
		BUTTON_C,
		BUTTON_X,
		BUTTON_Y,
		BUTTON_Z,
		BUTTON_L1,
		BUTTON_R1,
		BUTTON_L2,
		BUTTON_R2,
		BUTTON_THUMBL,
		BUTTON_THUMBR,
		BUTTON_START,
		BUTTON_SELECT,
		BUTTON_MODE,
		BUTTON_DPAD_UP,
		BUTTON_DPAD_DOWN,
		BUTTON_DPAD_LEFT,
		BUTTON_DPAD_RIGHT,
		BUTTON_DPAD_CENTER,
		BUTTON_BACK,

		BUTTON_COUNT
	};

	// this function MUST be called from the application's JNI_OnLoad,
	// from a function known to be called by JNI_OnLoad, or from a function
	// in a Java-called thread.
	static void			bindJavaMethods(JNIEnv *jniEnv, jobject classLoaderObject, jmethodID findClassMethod);

	VuAndroidGamePad();
	~VuAndroidGamePad();

	virtual bool init();
	virtual void postInit();
	virtual void preRelease();
	virtual void release();

	// cross-platform functionality
	virtual  VuController	&getController(int index) { return mpControllers[index]; }

	// platform-specific functionality
	static VuAndroidGamePad *IF() { return static_cast<VuAndroidGamePad *>(VuGamePad::IF()); }

	void			onAxisEvent(int deviceId, eDeviceType deviceType, float axisX, float axisY, float axisZ, float axisRX, float axisRY, float axisRZ, float axisLTrigger, float axisRTrigger, float axisGas, float axisBrake, float axisHatX, float axisHatY);
	void			onButtonEvent(int deviceId, eDeviceType deviceType, int button, bool down);

private:
	void			tick(float fdt);
	int				getPadIndex(int deviceId);

	// VuNearbyConnectionManager::Listener
	virtual void	onNCMessageReceived(const char *endpointId, const void *pData, int dataSize);

	class VuAndroidController : public VuController
	{
	public:
		VuAndroidController() : mDeviceId(-1) {}
		virtual void	playVibrationEffect(int effect) {}
		int				mDeviceId;
	};

	VuAndroidController		*mpControllers;
	float					mDisconnectTimer;
};
