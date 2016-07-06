//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 interface class for GamePad.
//
//*****************************************************************************

#pragma once

#if defined VUWIN32 || defined VUWINSTORE || defined VUWINPHONE
	#include <XInput.h>
#endif
#include "VuEngine/HAL/GamePad/VuGamePad.h"


class VuXInput : public VuGamePad
{
protected:
	virtual bool init();
	virtual void postInit();
	virtual void release();

public:
	VuXInput();
	~VuXInput();

	// cross-platform functionality
	virtual  VuController	&getController(int index) { return mpControllers[index]; }

protected:
	virtual void			tick(float fdt);
	virtual eDeviceType		identifyDeviceType(const XINPUT_CAPABILITIES &caps) = 0;

	void					detectNewlyConnectedPads(bool sendMsg);

	class VuXInputController : public VuController
	{
	public:
		VuXInputController();

		virtual void		playVibrationEffect(int effect);
		void				conditionByDeviceType();

		XINPUT_CAPABILITIES	mCaps;
		float				mLowFreq;
		float				mHighFreq;
	};

	VuXInputController		*mpControllers;
	float					mReconnectTimer;

	// Vibration
	bool					mbRumbleActive;
};
