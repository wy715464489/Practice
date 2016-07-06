//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  BB10 touchscreen hardware abstration layer
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Touch/VuTouch.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Containers/VuArray.h"


class VuBB10Touch : public VuTouch
{
public:
	VuBB10Touch();

	// public interface
	virtual int		getTouchCount(VUUINT32 priority) { return priority >= mFocusPriority ? mTouchArray.size() : 0; }
	virtual void	getTouch(int index, VuVector2 &touch) { touch = mTouchArray[index].mPos; }

	// platform-specific functionality
	static VuBB10Touch *IF() { return static_cast<VuBB10Touch *>(VuTouch::IF()); }

	// The following should be called by the appropriate event procedure.
	void		onTouchEvent(int type, VUUINT32 id, int x, int y);

private:
	struct VuTouchEntry
	{
		VUUINT32	mId;
		VuVector2	mPos;
	};
	typedef VuArray<VuTouchEntry> TouchArray;
	
	TouchArray	mTouchArray;
};
