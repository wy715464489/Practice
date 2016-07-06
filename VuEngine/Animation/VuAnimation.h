//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Animation class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Math/VuAabb.h"

class VuJsonContainer;
class VuBinaryDataReader;
class VuBinaryDataWriter;
class VuAnimationTransform;


class VuAnimation : public VuRefObj
{
public:
	VuAnimation();
protected:
	~VuAnimation();

public:
	bool			load(const VuJsonContainer &data, bool bAdditive = false);
	void			load(VuBinaryDataReader &reader);
	void			save(VuBinaryDataWriter &writer);

	int				getBoneCount() const			{ return mBoneCount; }
	int				getFrameCount() const			{ return mFrameCount; }
	float			getEndTime() const				{ return mEndTime; }	// time of last frame
	float			getTotalTime() const			{ return mTotalTime; }	// extra frame for looping
	const VuAabb	&getSkeletalLocalAabb() const	{ return mSkeletalLocalAabb; }
	bool			isAdditive() const				{ return mIsAdditive; }

	void			sample(float localTime, VuAnimationTransform *pPose) const;

private:
	void			endianSwap();
	void			buildDerivedData();

	int						mBoneCount;
	int						mFrameCount;
	VuAnimationTransform	*mpData; // framecount*bonecount
	float					mEndTime;
	float					mTotalTime;
	VuAabb					mSkeletalLocalAabb;
	bool					mIsAdditive;
};