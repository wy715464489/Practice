//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android accelerometer hardware abstration layer
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Accel/VuAccel.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Containers/VuArray.h"


class VuAndroidAccel : public VuAccel
{
public:
	VuAndroidAccel();

	// public interface
	virtual bool	getAccel(VuVector3 &accel) { accel = mAccel; return mIsActive; }

	// platform-specific functionality
	static VuAndroidAccel *IF() { return static_cast<VuAndroidAccel *>(VuAccel::IF()); }

	// The following should be called by the appropriate event procedure.
	void	onGravityEvent(const VuVector3 &rawVector);
	void	onAccelEvent(const VuVector3 &rawAccel);

private:
	struct VuEntry
	{
		VuVector3	mValue;
		int			mTimeMS;
	};
	typedef VuArray<VuEntry> History;
	History		mHistory;
	VuVector3	mAccel;
	bool		mIsActive;
};
