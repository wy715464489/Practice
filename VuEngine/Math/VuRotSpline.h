//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Rotational Spline class
//
//*****************************************************************************

#pragma once

#include "VuQuaternion.h"
#include "VuEngine/Containers/VuArray.h"


class VuRotSpline
{
public:
	VuRotSpline();

	struct Key
	{
		VuQuaternion	mRot;
		float			mTime;
	};

	bool			build(Key *keys, int count);
	void			clear();
	bool			isBuilt() const	{ return mIsBuilt; }

	VuQuaternion	getRotationAtTime(float time);

private:
	struct Poly
	{
		VuQuaternion	getRotation(float u);

		float			mTimeMin;
		float			mTimeMax;
		float			mTimeInvRange;

		VuQuaternion	mP;
		VuQuaternion	mA;
		VuQuaternion	mB;
		VuQuaternion	mQ;
	};

	VuArray<Poly>	mPolyArray;
	bool			mIsBuilt;
};
