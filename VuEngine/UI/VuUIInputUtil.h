//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI input utility functionality.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/GamePad/VuGamePad.h"
#include "VuEngine/HAL/Touch/VuTouch.h"

class VuEntity;


class VuUIInputUtil : public VuTouch::Callback
{
public:
	enum GamePadChannel
	{
		CHANNEL_SELECT,
		CHANNEL_BACK,
		CHANNEL_UP,
		CHANNEL_DOWN,
		CHANNEL_LEFT,
		CHANNEL_RIGHT,

		NUM_CHANNELS,
	};

	enum TouchInput
	{
		TOUCH_DOWN,
		TOUCH_UP,
		TOUCH_MOVE,
	};

	VuUIInputUtil(VuEntity *pOwner);
	~VuUIInputUtil();

	void			enable();
	void			disable();
	void			setPriority(VUUINT32 priority);
	void			tick(VUUINT32 padMask);

protected:
	// VuTouch::Callback
	virtual void	onTouchDown(const VuVector2 &touch);
	virtual void	onTouchUp(const VuVector2 &touch);
	virtual void	onTouchMove();
	virtual void	onTouchSpecial(VuTouch::eSpecial special);

	void			sendGamePadEvent(int channel, bool down, int padIndex);

	VuEntity		*mpOwner;
	bool			mEnabled;
	VUUINT32		mPriority;
	float			mRepeatDelay;
	float			mRepeatTimer;
	float			mRepeatTimers[VuGamePad::MAX_NUM_PADS][NUM_CHANNELS];
};