//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  BB10 accelerometer hardware abstration layer
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Accel/VuAccel.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Containers/VuArray.h"


class VuBB10Accel : public VuAccel
{
public:
	VuBB10Accel();

	// public interface
	virtual bool	getAccel(VuVector3 &accel) { accel = mAccel; return mIsActive; }

	// platform-specific functionality
	static VuBB10Accel *IF() { return static_cast<VuBB10Accel *>(VuAccel::IF()); }

	// The following should be called by the appropriate event procedure.
	void	onAccel(const VuVector3 &rawAccel);

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
