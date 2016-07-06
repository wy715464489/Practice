//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Scrolling Text class
// 
//*****************************************************************************

#pragma once

#include "VuUITextEntity.h"


class VuUIScrollingTextEntity : public VuUITextEntity
{
	DECLARE_RTTI

public:
	VuUIScrollingTextEntity();
	virtual void		onGameInitialize();

protected:
	// event handlers
	void				OnUITick(const VuParams &params);

	// scripting
	VuRetVal			StartScroll(const VuParams &params);
	VuRetVal			StopScroll(const VuParams &params);
	VuRetVal			ResetScroll(const VuParams &params);

	// properties
	bool				mbScrollAtStart;
	float				mScrollSpeed;

	bool				mbScrolling;
	bool				mbReachedEnd;
};
