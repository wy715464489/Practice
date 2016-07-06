//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ps4 interface class for GamePad.
//
//*****************************************************************************
#pragma once

#include <pad.h>

#include "VuEngine/HAL/GamePad/VuGamePad.h"

class VuPs4GamePad : public VuGamePad
{
public:
	VuPs4GamePad();

	virtual bool			init();
	virtual void			postInit();
	virtual void			release();

	virtual VuController	&getController(int index) { return mControllers[index]; }

	static VuPs4GamePad		*IF() { return static_cast<VuPs4GamePad *>(VuGamePad::IF()); }

private:
	void					tick(float fdt);

	VUINT					findVuController(SceUserServiceUserId userId);
	void					addVuController(SceUserServiceUserId userId, int handle, eDeviceType deviceType, float deadZoneLeft, float deadZoneRight);
	void					removeVuController(SceUserServiceUserId userId);

	void					inspectPs4Controller(SceUserServiceUserId userId);
	void					inspectPs4Controllers();


	class VuPs4Controller : public VuController
	{
	public:
		virtual void			playVibrationEffect(int effect);

		ScePadData				mData;
		int						mHandle;
		SceUserServiceUserId	mUserId;
		float					mDeadZoneLeft;
		float					mDeadZoneRight;
		float					mHighFreq;
		float					mLowFreq;
	};

	// Ps4 controllers
	VuPs4Controller		mControllers[MAX_NUM_PADS];
	
	// Vibration
	bool				mRumbleActive;
};

