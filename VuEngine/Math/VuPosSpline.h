//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Positional Spline class
//
//*****************************************************************************

#pragma once

#include "VuVector3.h"
#include "VuEngine/Containers/VuArray.h"


class VuPosSpline
{
public:
	VuPosSpline();

	struct Key
	{
		VuVector3	mPos;
		float		mTime;
	};

	bool			build(Key *keys, int count);
	void			clear();
	bool			isBuilt() const	{ return mIsBuilt; }

	VuVector3		getPositionAtTime(float time);
	VuVector3		getVelocityAtTime(float time);
	VuVector3		getAccelerationAtTime(float time);

	float			getLength(float time);
	float			getTotalLength() { return mTotalLength; }

	VuVector3		getPositionAtLength(float length);
	VuVector3		getVelocityAtLength(float length);
	VuVector3		getAccelerationAtLength(float length);

private:
	struct Poly
	{
		VuVector3	getPosition(float u);
		VuVector3	getVelocity(float u);
		VuVector3	getAcceleration(float u);
		float		getSpeed(float u);
		float		getLength(float u);

		float		mTimeMin;
		float		mTimeMax;
		float		mTimeInvRange;

		VuVector3	mC[4];
	};

	void			findPoly(float time, int &i, float &u);
	void			invertIntegral(float length, int &i, float &u);

	VuArray<Poly>	mPolyArray;
	VuArray<float>	mLengthArray;
	float			mTotalLength;
	bool			mIsBuilt;
};
