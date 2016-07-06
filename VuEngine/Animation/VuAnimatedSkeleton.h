//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animated Skeleton class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Math/VuAabb.h"

class VuSkeleton;
class VuAnimationTransform;
class VuAnimationControl;


class VuAnimatedSkeleton : public VuRefObj
{
public:
	VuAnimatedSkeleton(VuSkeleton *pSkeleton);
protected:
	~VuAnimatedSkeleton();

public:
	void						advance(float fdt);
	void						build();

	void						addAnimationControl(VuAnimationControl *pAnimationControl);
	void						removeAnimationControl(VuAnimationControl *pAnimationControl);
	void						clearAnimationControls();
	void						clearBlendAnimationControls();
	void						clearAdditiveAnimationControls();

	const VuSkeleton			*getSkeleton() const					{ return mpSkeleton; }
	const VuAnimationTransform	*getLocalPose() const					{ return mpLocalPose; }
	const VuAabb				&getSkeletalLocalAabb() const			{ return mSkeletalLocalAabb; }

	int							getAnimationControlCount() const		{ return mAnimationControls.size(); }
	VuAnimationControl			*getAnimationControl(int index) const	{ return mAnimationControls[index]; }

	int							getBlendAnimationControlCount() const		{ return mBlendAnimationControls.size(); }
	VuAnimationControl			*getBlendAnimationControl(int index) const	{ return mBlendAnimationControls[index]; }

	int							getAdditiveAnimationControlCount() const		{ return mAdditiveAnimationControls.size(); }
	VuAnimationControl			*getAdditiveAnimationControl(int index) const	{ return mAdditiveAnimationControls[index]; }

	void						drawInfo(const VuVector3 &pos) const;

protected:
	typedef VuArray<VuAnimationControl *> AnimationControls;

	VuSkeleton					*mpSkeleton;
	AnimationControls			mAnimationControls;
	AnimationControls			mBlendAnimationControls;
	AnimationControls			mAdditiveAnimationControls;
	VuAnimationTransform		*mpLocalPose;
	VuAabb						mSkeletalLocalAabb;
};