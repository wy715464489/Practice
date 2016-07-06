//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuTransitionComponent
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Components/VuComponent.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Method/VuParams.h"


class VuTransitionBaseComponent : public VuComponent
{
	DECLARE_RTTI

public:
	VuTransitionBaseComponent(VuEntity *pOwnerEntity);

	virtual void	tick(float fdt) = 0;

	virtual void	transitionIn(bool force = false) = 0;
	virtual void	transitionOut(bool force = false) = 0;

	bool			isTransitioning() { return mState == STATE_TRANS_IN || mState == STATE_TRANS_OUT; }

	enum eState { STATE_OFF, STATE_TRANS_IN, STATE_ACTIVE, STATE_TRANS_OUT };
	eState			getState() { return mState; }

protected:
	// scripting
	VuRetVal		TransitionIn(const VuParams &params) { transitionIn(true); return VuRetVal(); }
	VuRetVal		TransitionOut(const VuParams &params) { transitionOut(true); return VuRetVal(); }

	enum { BEHAVIOR_AUTOMATIC, BEHAVIOR_MANUAL_IN, BEHAVIOR_MANUAL_OUT };

	// properties
	float			mTransitionDuration;
	int				mBehavior;

	eState			mState;
};


class VuTransitionComponent : public VuTransitionBaseComponent
{
	DECLARE_SHORT_COMPONENT_TYPE(Transition)
	DECLARE_RTTI

public:
	VuTransitionComponent(VuEntity *pOwnerEntity);

	virtual void	onPostLoad();
	virtual void	onGameInitialize();

	virtual void	tick(float fdt);

	virtual void	transitionIn(bool force);
	virtual void	transitionOut(bool force);

	float			alpha() { return mAlpha; }

private:
	enum { TRANS_NONE, TRANS_FADE, TRANS_OFFSET, TRANS_BOING, TRANS_OFFSET_BOUNCE };

	void			setStateOff();
	void			setStateActive();

	// properties
	int				mTransitionType;
	VuVector2		mHiddenOffset;
	float			mAngularFrequency;
	float			mDampingRatio;

	VuVector2		mBasePosition;

	float			mProgress;
	float			mAlpha;
	float			mSpringTarget;
	float			mSpringPos;
	float			mSpringVel;
	bool			mSpringActive;
};
