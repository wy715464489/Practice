//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 touchscreen hardware abstration layer
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Touch/VuTouch.h"
#include "VuEngine/Math/VuVector2.h"


class VuWin32Touch : public VuTouch
{
public:
	VuWin32Touch();

	// public interface
	virtual int		getTouchCount(VUUINT32 priority) { return priority >= mFocusPriority ? mMouseDown : 0; }
	virtual void	getTouchRaw(int index, VuVector2 &touch) { touch = mMousePos; }

	// platform-specific functionality
	static VuWin32Touch *IF() { return static_cast<VuWin32Touch *>(VuTouch::IF()); }

	// The following should be called by the appropriate event procedure.
	void		onMouseEvent(int type, float x1, float y1);

private:
	int			mMouseDown;
	VuVector2	mMousePos;
};
