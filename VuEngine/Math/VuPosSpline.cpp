//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Positional Spline class
//
//*****************************************************************************

#include "VuPosSpline.h"


//*****************************************************************************
VuPosSpline::VuPosSpline():
	mPolyArray(0),
	mLengthArray(0),
	mTotalLength(0.0f),
	mIsBuilt(false)
{
}

//*****************************************************************************
bool VuPosSpline::build(Key *keys, int count)
{
	if ( count < 4 )
		return false;

	mPolyArray.resize(count - 3);
	for (int i0=0, i1=1, i2=2, i3=3; i0 < mPolyArray.size(); i0++, i1++, i2++, i3++)
	{
		VuVector3 kDiff10 = keys[i1].mPos - keys[i0].mPos;
		VuVector3 kDiff21 = keys[i2].mPos - keys[i1].mPos;
		VuVector3 kDiff32 = keys[i3].mPos - keys[i2].mPos;

		// build multipliers at point P[i1]
		float fOmT0 = 1.0f - 0.0f;//keys[i1].mTension;
		float fOmC0 = 1.0f - 0.0f;//keys[i1].mContinuity;
		float fOpC0 = 1.0f + 0.0f;//keys[i1].mContinuity;
		float fOmB0 = 1.0f - 0.0f;//keys[i1].mBias;
		float fOpB0 = 1.0f + 0.0f;//keys[i1].mBias;
		float fAdj0 = 2.0f*(keys[i2].mTime - keys[i1].mTime)/(keys[i2].mTime - keys[i0].mTime);
		float fOut0 = 0.5f*fAdj0*fOmT0*fOpC0*fOpB0;
		float fOut1 = 0.5f*fAdj0*fOmT0*fOmC0*fOmB0;

		// build outgoing tangent at P[i1]
		VuVector3 kTOut = fOut1*kDiff21 + fOut0*kDiff10;

		// build multipliers at point P[i2]
		float fOmT1 = 1.0f - 0.0f;//keys[i2].mTension;
		float fOmC1 = 1.0f - 0.0f;//keys[i2].mContinuity;
		float fOpC1 = 1.0f + 0.0f;//keys[i2].mContinuity;
		float fOmB1 = 1.0f - 0.0f;//keys[i2].mBias;
		float fOpB1 = 1.0f + 0.0f;//keys[i2].mBias;
		float fAdj1 = 2.0f*(keys[i2].mTime - keys[i1].mTime)/(keys[i3].mTime - keys[i1].mTime);
		float fIn0 = 0.5f*fAdj1*fOmT1*fOmC1*fOpB1;
		float fIn1 = 0.5f*fAdj1*fOmT1*fOpC1*fOmB1;

		// build incoming tangent at P[i2]
		VuVector3 kTIn = fIn1*kDiff32 + fIn0*kDiff21;
		mPolyArray[i0].mC[0] = keys[i1].mPos;
		mPolyArray[i0].mC[1] = kTOut;
		mPolyArray[i0].mC[2] = 3.0f*kDiff21 - 2.0f*kTOut - kTIn;
		mPolyArray[i0].mC[3] = -2.0f*kDiff21 + kTOut + kTIn;
		mPolyArray[i0].mTimeMin = keys[i1].mTime;
		mPolyArray[i0].mTimeMax = keys[i2].mTime;
		mPolyArray[i0].mTimeInvRange = 1.0f/(keys[i2].mTime - keys[i1].mTime);
	}

	// compute arc lengths of polynomials and total length of spline
	mLengthArray.resize(mPolyArray.size() + 1);
	mLengthArray[0] = 0.0f;
	for (int i = 0; i < mPolyArray.size(); i++)
	{
		// length of current polynomial
		float fPolyLength = mPolyArray[i].getLength(1.0f);

		// total length of curve between poly[0] and poly[i+1]
		mLengthArray[i+1] = mLengthArray[i] + fPolyLength;
	}

	mTotalLength = mLengthArray[mPolyArray.size()];

	mIsBuilt = true;

	return true;
}

//*****************************************************************************
void VuPosSpline::clear()
{
	mPolyArray.deallocate();
	mLengthArray.deallocate();
	mTotalLength = 0.0f;
	mIsBuilt = false;
}

//*****************************************************************************
VuVector3 VuPosSpline::getPositionAtTime(float time)
{
	int i;
	float u;
	findPoly(time, i, u);

	return mPolyArray[i].getPosition(u);
}

//*****************************************************************************
VuVector3 VuPosSpline::getVelocityAtTime(float time)
{
	int i;
	float u;
	findPoly(time, i, u);

	return mPolyArray[i].getVelocity(u);
}

//*****************************************************************************
VuVector3 VuPosSpline::getAccelerationAtTime(float time)
{
	int i;
	float u;
	findPoly(time, i, u);

	return mPolyArray[i].getAcceleration(u);
}

//*****************************************************************************
float VuPosSpline::getLength(float time)
{
	int i;
	float u;
	findPoly(time, i, u);

	return mPolyArray[i].getLength(u);
}

//*****************************************************************************
VuVector3 VuPosSpline::getPositionAtLength(float length)
{
	int i;
	float u;
	invertIntegral(length, i, u);

	return mPolyArray[i].getPosition(u);
}

//*****************************************************************************
VuVector3 VuPosSpline::getVelocityAtLength(float length)
{
	int i;
	float u;
	invertIntegral(length, i, u);

	return mPolyArray[i].getVelocity(u);
}

//*****************************************************************************
VuVector3 VuPosSpline::getAccelerationAtLength(float length)
{
	int i;
	float u;
	invertIntegral(length, i, u);

	return mPolyArray[i].getAcceleration(u);
}

//*****************************************************************************
void VuPosSpline::findPoly(float time, int &i, float &u)
{
	// Lookup the polynomial that contains the input time in its domain of
	// evaluation. Clamp to [tmin,tmax].
	if ( mPolyArray[0].mTimeMin < time )
	{
		if ( time < mPolyArray.back().mTimeMax )
		{
			for (i = 0; i < mPolyArray.size(); i++)
			{
				if ( time < mPolyArray[i].mTimeMax )
					break;
			}
			u = (time - mPolyArray[i].mTimeMin)*mPolyArray[i].mTimeInvRange;
		}
		else
		{
			i = mPolyArray.size() - 1;
			u = 1.0f;
		}
	}
	else
	{
		i = 0;
		u = 0.0f;
	}
}

//*****************************************************************************
void VuPosSpline::invertIntegral(float length, int &i, float &u)
{
	// clamp s to [0,L] so that t in [tmin,tmax]
	if ( length <= 0.0f )
	{
		i = 0;
		u = 0.0f;
		return;
	}
	if ( length >= mTotalLength )
	{
		i = mPolyArray.size() - 1;
		u = 1.0f;
		return;
	}

	// determine which polynomial corresponds to s
	float fDist=0.0f;
	for (i = 0; i < mPolyArray.size(); i++)
	{
		if ( length <= mLengthArray[i+1] )
		{
			// distance along segment
			fDist = length - mLengthArray[i];
			// initial guess for inverting the arc length integral
			u = fDist/(mLengthArray[i+1] - mLengthArray[i]);
			break;
		}
	}

	// use Newton's method to invert the arc length integral
	const float fTolerance = 1e-06f;
	const int jMax = 32;
	for (int j = 0; j < jMax; j++)
	{
		float fDiff = mPolyArray[i].getLength(u) - fDist;
		if ( VuAbs(fDiff) <= fTolerance )
			break;

		// assert: speed > 0
		u -= fDiff/mPolyArray[i].getSpeed(u);
	}
}

//*****************************************************************************
VuVector3 VuPosSpline::Poly::getPosition(float u)
{
	VuVector3 kResult = mC[0] + u*(mC[1] + u*(mC[2] + u*mC[3]));

	return kResult;
}

//*****************************************************************************
VuVector3 VuPosSpline::Poly::getVelocity(float u)
{
	VuVector3 kResult = mC[1] + u*(2.0f*mC[2] + 3.0f*u*mC[3]);

	return kResult;
}

//*****************************************************************************
VuVector3 VuPosSpline::Poly::getAcceleration(float u)
{
	VuVector3 kResult = 2.0f*mC[2] + 6.0f*u*mC[3];

	return kResult;
}

//*****************************************************************************
float VuPosSpline::Poly::getSpeed(float u)
{
	return getVelocity(u).mag();
}

//*****************************************************************************
float VuPosSpline::Poly::getLength(float u)
{
	static float sModRoot[5] = { 0.046910077f, 0.230765345f, 0.5f, 0.769234655f, 0.953089922f };
	static float sModCoeff[5] = { 0.118463442f, 0.239314335f, 0.284444444f, 0.239314335f, 0.118463442f };

	// Need to transform domain [0,u] to [-1,1]. If 0 <= x <= u
	// and -1 <= t <= 1, then x = u*(t+1)/2.
	float fResult = 0.0f;

	for (int i = 0; i < 5; i++)
		fResult += sModCoeff[i]*getSpeed(u*sModRoot[i]);

	fResult *= u;

	return fResult;
}
