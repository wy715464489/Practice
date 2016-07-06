//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Windows touchscreen hardware abstration layer
//
//*****************************************************************************

#pragma once

#include "VuEngine/Events/VuEventMap.h"
#include "VuEngine/HAL/Touch/VuTouch.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Containers/VuArray.h"


class VuWindowsTouch : public VuTouch
{
	DECLARE_EVENT_MAP

public:
	VuWindowsTouch();

	// public interface
	virtual int		getTouchCount(VUUINT32 priority) { return priority >= mFocusPriority ? mTouchPoints.size() : 0; }
	virtual void	getTouchRaw(int index, VuVector2 &touch) { touch = mTouchPoints[index].mPos; }

	// platform-specific functionality
	static VuWindowsTouch *IF() { return static_cast<VuWindowsTouch *>(VuTouch::IF()); }

	// The following should be called by the appropriate event procedure.
	void		onPointerDown(VUUINT32 id, float x1, float y1);
	void		onPointerUp(VUUINT32 id, float x1, float y1);
	void		onPointerMove(VUUINT32 id, float x1, float y1);
	void		onTouchSpecial(eSpecial special);

private:
	// event handlers
	void		OnWindowsPointerDown(const VuParams &params);
	void		OnWindowsPointerUp(const VuParams &params);
	void		OnWindowsPointerMove(const VuParams &params);
	void		OnWindowsTouchSpecial(const VuParams &params);

	struct TouchPoint
	{
		VUUINT32	mID;
		VuVector2	mPos;
	};
	typedef VuArray<TouchPoint> TouchPoints;

	TouchPoint	*findTouchPoint(VUUINT32 id);

	TouchPoints	mTouchPoints;
};
