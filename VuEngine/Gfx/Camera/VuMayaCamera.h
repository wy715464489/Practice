//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  MayaCamera class
// 
//*****************************************************************************

#pragma once

#include "VuCamera.h"
#include "VuEngine/Math/VuVector2.h"


class VuMayaCamera : public VuCamera
{
public:
	VuMayaCamera();

	enum {
		BUTTON_NONE		= (0<<0),
		BUTTON_LEFT		= (1<<0),
		BUTTON_MIDDLE	= (1<<1),
		BUTTON_RIGHT	= (1<<2),
	};
	enum {
		MODIFIER_NONE		= (0<<0),
		MODIFIER_SHIFT		= (1<<0),
		MODIFIER_CONTROL	= (1<<1),
		MODIFIER_ALT		= (1<<2),
	};

	void	onButtonDown(VUINT x, VUINT y)	{ mLastMouseX = x; mLastMouseY = y; }
	bool	onMouseMove(VUINT x, VUINT y, VUINT dw, VUUINT32 buttons, VUUINT32 modifiers);

	void	startMovement(VUUINT32 dir);
	void	stopMovement(VUUINT32 dir);

	void	setDisplaySize(const VuVector2 &size) { mDisplaySize = size; }
	bool	tick();
	void	find(const VuVector3 &vTargetPos, float fTargetSize);

	enum {
		MOVE_POS_X = (1<<0),
		MOVE_POS_Y = (1<<1),
		MOVE_POS_Z = (1<<2),
		MOVE_NEG_X = (1<<3),
		MOVE_NEG_Y = (1<<4),
		MOVE_NEG_Z = (1<<5),
	};

private:
	void		update();

	VuVector2	mDisplaySize;

	VuVector3	mTarget;
	float		mDist;
	float		mRotX;
	float		mRotZ;

//	VUUINT32	mButtons;
	VUINT		mLastMouseX;
	VUINT		mLastMouseY;
	VUUINT32	mMovement;
	double		mLastTime;
};