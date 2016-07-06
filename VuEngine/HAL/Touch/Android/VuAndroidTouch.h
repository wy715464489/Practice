//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Android touchscreen hardware abstration layer
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Touch/VuTouch.h"
#include "VuEngine/Math/VuVector2.h"


class VuAndroidTouch : public VuTouch
{
public:
	VuAndroidTouch();

	// public interface
	virtual int		getTouchCount(VUUINT32 priority) { return priority >= mFocusPriority ? mTouchCount : 0; }
	virtual void	getTouchRaw(int index, VuVector2 &touch) { touch = mTouchArray[index]; }

	// platform-specific functionality
	static VuAndroidTouch *IF() { return static_cast<VuAndroidTouch *>(VuTouch::IF()); }

	// The following should be called by the appropriate event procedure.
	void		onTouchEvent(int action, int pointerBits, float x1, float y1, float x2, float y2);
	void		onTouchSpecial(eSpecial special);

private:
	struct Pointer
	{
		bool		mDown;
		VuVector2	mPos;
	};

	Pointer		mPointers[2];
	VuVector2	mTouchArray[2];
	int			mTouchCount;
};
