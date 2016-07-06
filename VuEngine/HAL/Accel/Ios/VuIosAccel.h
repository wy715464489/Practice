//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ios accelerometer hardware abstration layer
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Accel/VuAccel.h"
#include "VuEngine/Math/VuVector3.h"


class VuIosAccel : public VuAccel
{
public:
	VuIosAccel();

	// public interface
	virtual bool	getAccel(VuVector3 &accel) { accel = mAccel; return true; }

	// platform-specific functionality
	static VuIosAccel *IF() { return static_cast<VuIosAccel *>(VuAccel::IF()); }

	// The following should be called by the appropriate event procedure.
	void	onAccel(float accX, float accY, float accZ);

private:
	enum { HISTORY_SIZE = 10 };
	VuVector3	mHistory[HISTORY_SIZE];
	int			mHistoryIndex;
	VuVector3	mAccel;
};
