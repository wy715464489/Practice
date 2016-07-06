//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animation Control class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuAnimationEventIF.h"

class VuAnimation;
class VuAnimationAsset;
class VuTimedEventAsset;


class VuAnimationControl : public VuRefObj
{
public:
	VuAnimationControl(const std::string &assetName);
	VuAnimationControl(VuAnimation *pAnimation);
protected:
	~VuAnimationControl();
public:

	void				setLooping(bool bLooping);
	bool				getLooping() { return mbLooping; }
	void				setTimeFactor(float timeFactor)	{ mTimeFactor = timeFactor; }
	float				getTimeFactor()					{ return mTimeFactor; }

	void				advance(float fdt);

	void				setLocalTime(float time);
	float				getLocalTime()	{ return mLocalTime; }
	bool				isAtStart() const;
	bool				isAtEnd() const;

	void				setWeight(float weight)	{ mWeight = weight; }
	float				getWeight()				{ return mWeight; }

	// timed events
	void				setTimedEventAsset(VuTimedEventAsset *pTimedEventAsset);
	void				setEventIF(VuAnimationEventIF *pIF)	{ mpEventIF = pIF; }

	const VuAnimation	*getAnimation()	const { return mpAnimation; }

protected:
	void				handleTimedEventsForward(float prevTime, float curTime);
	void				handleTimedEventsReverse(float prevTime, float curTime);

	VuAnimationAsset	*mpAnimationAsset;
	VuAnimation			*mpAnimation;
	bool				mbLooping;
	float				mTimeFactor;
	float				mLocalTime;
	float				mWeight;
	VuTimedEventAsset	*mpTimedEventAsset;
	VuAnimationEventIF	*mpEventIF;
};