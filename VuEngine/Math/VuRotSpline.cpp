//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Rotational Spline class
//
//*****************************************************************************

#include "VuRotSpline.h"


//*****************************************************************************
VuRotSpline::VuRotSpline():
	mPolyArray(0),
	mIsBuilt(false)
{
}

//*****************************************************************************
bool VuRotSpline::build(Key *keys, int count)
{
	if ( count < 4 )
		return false;

	mPolyArray.resize(count - 3);

	// Consecutive quaterions should form an acute angle. Changing sign on
	// a quaternion does not change the rotation it represents.
	int i;
	for (i = 1; i < count; i++)
	{
		if ( VuDot(keys[i].mRot.mVec, keys[i-1].mRot.mVec) < 0.0f )
			keys[i].mRot.mVec = -keys[i].mRot.mVec;
	}

	for (int i0=0,i1=1,i2=2,i3=3; i0 < mPolyArray.size(); i0++,i1++,i2++,i3++)
	{
		VuQuaternion kQ0 = keys[i0].mRot;
		VuQuaternion kQ1 = keys[i1].mRot;
		VuQuaternion kQ2 = keys[i2].mRot;
		VuQuaternion kQ3 = keys[i3].mRot;
		VuQuaternion kLog10 = (kQ0.conjugate()*kQ1).log();
		VuQuaternion kLog21 = (kQ1.conjugate()*kQ2).log();
		VuQuaternion kLog32 = (kQ2.conjugate()*kQ3).log();

		// build multipliers at q[i1]
		float fOmT0 = 1.0f - 0.0f;//keys[i1].Tension();
		float fOmC0 = 1.0f - 0.0f;//keys[i1].Continuity();
		float fOpC0 = 1.0f + 0.0f;//keys[i1].Continuity();
		float fOmB0 = 1.0f - 0.0f;//keys[i1].Bias();
		float fOpB0 = 1.0f + 0.0f;//keys[i1].Bias();
		float fAdj0 = 2.0f*(keys[i2].mTime - keys[i1].mTime)/(keys[i2].mTime - keys[i0].mTime);
		float fOut0 = 0.5f*fAdj0*fOmT0*fOpC0*fOpB0;
		float fOut1 = 0.5f*fAdj0*fOmT0*fOmC0*fOmB0;

		// build outgoing tangent at q[i1]
		VuQuaternion kTOut = fOut1*kLog21 + fOut0*kLog10;

		// build multipliers at q[i2]
		float fOmT1 = 1.0f - 0.0f;//keys[i2].Tension();
		float fOmC1 = 1.0f - 0.0f;//keys[i2].Continuity();
		float fOpC1 = 1.0f + 0.0f;//keys[i2].Continuity();
		float fOmB1 = 1.0f - 0.0f;//keys[i2].Bias();
		float fOpB1 = 1.0f + 0.0f;//keys[i2].Bias();
		float fAdj1 = 2.0f*(keys[i2].mTime - keys[i1].mTime)/(keys[i3].mTime - keys[i1].mTime);
		float fIn0 = 0.5f*fAdj1*fOmT1*fOmC1*fOpB1;
		float fIn1 = 0.5f*fAdj1*fOmT1*fOpC1*fOmB1;

		// build incoming tangent at q[i2]
		VuQuaternion kTIn = fIn1*kLog32 + fIn0*kLog21;
		mPolyArray[i0].mP = kQ1;
		mPolyArray[i0].mQ = kQ2;
		mPolyArray[i0].mA = kQ1*((0.5f*(kTOut-kLog21)).exp());
		mPolyArray[i0].mB = kQ2*((0.5f*(kLog21-kTIn)).exp());
		mPolyArray[i0].mTimeMin = keys[i1].mTime;
		mPolyArray[i0].mTimeMax = keys[i2].mTime;
		mPolyArray[i0].mTimeInvRange = 1.0f/(keys[i2].mTime-keys[i1].mTime);
	}

	mIsBuilt = true;

	return true;
}

//*****************************************************************************
void VuRotSpline::clear()
{
	mPolyArray.deallocate();
	mIsBuilt = false;
}

//*****************************************************************************
VuQuaternion VuRotSpline::getRotationAtTime(float time)
{
	// find the interpolating polynomial (clamping used, modify for looping)
	int i;
	float u;
	if ( mPolyArray[0].mTimeMin < time )
	{
		if ( time < mPolyArray[mPolyArray.size()-1].mTimeMax )
		{
			for (i = 0; i < mPolyArray.size(); i++)
			{
				if ( time < mPolyArray[i].mTimeMax )
					break;
			}
			u = (time-mPolyArray[i].mTimeMin)*mPolyArray[i].mTimeInvRange;
		}
		else
		{
			i = mPolyArray.size()-1;
			u = 1.0f;
		}
	}
	else
	{
		i = 0;
		u = 0.0f;
	}

	return mPolyArray[i].getRotation(u);
}

//*****************************************************************************
VuQuaternion VuRotSpline::Poly::getRotation(float u)
{
	VuQuaternion squad = VuSquad(mP, mQ, mA, mB, u);

	return squad;
}