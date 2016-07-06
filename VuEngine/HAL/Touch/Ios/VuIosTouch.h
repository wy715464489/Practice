//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ios touchscreen hardware abstration layer
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Touch/VuTouch.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Containers/VuArray.h"


class VuIosTouch : public VuTouch
{
public:
	VuIosTouch();

	// public interface
	virtual int		getTouchCount(VUUINT32 priority) { return priority >= mFocusPriority ? mTouchArray.size() : 0; }
	virtual void	getTouchRaw(int index, VuVector2 &touch) { touch = mTouchArray[index].mPos; }

	// platform-specific functionality
	static VuIosTouch *IF() { return static_cast<VuIosTouch *>(VuTouch::IF()); }

	// The following should be called by the appropriate event procedure.
	void		onTouchDown(void *pTouch, float x, float y);
	void		onTouchMove(void *pTouch, float x, float y);
	void		onTouchUp(void *pTouch, float x, float y);
	void		onTouchCancel(void *pTouch, float x, float y);

private:
	struct VuTouchEntry
	{
		void		*mpTouch;
		VuVector2	mPos;
	};
	typedef VuArray<VuTouchEntry> TouchArray;
	
	TouchArray	mTouchArray;
};
