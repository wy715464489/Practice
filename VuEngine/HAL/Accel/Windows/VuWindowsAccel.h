//*****************************************************************************
//
//  Copyright (c) 2011-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Windows accelerometer hardware abstration layer
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Accel/VuAccel.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Containers/VuArray.h"


class VuWindowsAccel : public VuAccel
{
public:
	VuWindowsAccel();

	// public interface
	virtual bool	getAccel(VuVector3 &accel) { accel = mAccel; return mIsActive; }

	// platform-specific functionality
	static VuWindowsAccel *IF() { return static_cast<VuWindowsAccel *>(VuAccel::IF()); }

	// The following should be called by the appropriate event procedure.
	void	onGravity(const VuVector3 &rawVector);
	void	onAccel(const VuVector3 &accel);

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
