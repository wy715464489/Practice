//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Cubic Curve class
//
//  [Numerical Recipes in C, the Art of Scientific Computing (2nd Edition)]
// 
//*****************************************************************************

#include "VuCubicCurve.h"
#include "VuEngine/Memory/VuScratchPad.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuMathUtil.h"


//*****************************************************************************
VuCubicPosCurve::VuCubicPosCurve():
	mControlPoints(0),
	mIsBuilt(false)
{
}

//*****************************************************************************
void VuCubicPosCurve::clear()
{
	mControlPoints.clear();
	mIsBuilt = false;
}

//*****************************************************************************
void VuCubicPosCurve::reserve(int count)
{
	mControlPoints.reserve(count);
}

//*****************************************************************************
void VuCubicPosCurve::addControlPoint(const VuVector3 &pos, float fTime)
{
	mControlPoints.resize(mControlPoints.size() + 1);
	mControlPoints.back().mPos = pos;
	mControlPoints.back().mTime = fTime;
}

//*****************************************************************************
void VuCubicPosCurve::sort()
{
	qsort(&mControlPoints[0], mControlPoints.size(), sizeof(mControlPoints[0]), compareControlPoints);
}

//*****************************************************************************
bool VuCubicPosCurve::build(const VuVector3 &derivStart, const VuVector3 &derivEnd)
{
	int count = mControlPoints.size();

	// verify
	if ( count < 2 )
		return false;

	// numerical recipes uses float arrays, so we will use 
	// scratch pad to translate
	float *t = (float *)VuScratchPad::get();
	float *u = t + count;
	float *y = u + count;
	float *y2 = y + count;

	// load times onto scratch pad
	for ( int i = 0; i < count; i++ )
		t[i] = mControlPoints[i].mTime;

	// x coordinate
	for ( int i = 0; i < count; i++ )
		y[i] = mControlPoints[i].mPos.mX;
	spline(t, y, count, derivStart.mX, derivEnd.mX, y2, u );
	for ( int i = 0; i < count; i++ )
		mControlPoints[i].mVel.mX = y2[i];

	// y coordinate
	for ( int i = 0; i < count; i++ )
		y[i] = mControlPoints[i].mPos.mY;
	spline(t, y, count, derivStart.mY, derivEnd.mY, y2, u );
	for ( int i = 0; i < count; i++ )
		mControlPoints[i].mVel.mY = y2[i];

	// z coordinate
	for ( int i = 0; i < count; i++ )
		y[i] = mControlPoints[i].mPos.mZ;
	spline(t, y, count, derivStart.mZ, derivEnd.mZ, y2, u );
	for ( int i = 0; i < count; i++ )
		mControlPoints[i].mVel.mZ = y2[i];

	mIsBuilt = true;

	return true;
}

//*****************************************************************************
void VuCubicPosCurve::getPointAtTime(float fTime, VuVector3 &pos) const
{
	if ( fTime <= mControlPoints.begin().mTime )
	{
		pos = mControlPoints.begin().mPos;
		return;
	}

	if ( fTime >= mControlPoints.back().mTime )
	{
		pos = mControlPoints.back().mPos;
		return;
	}

	// find index
	int index;
	for ( index = 1; index < mControlPoints.size(); index++ )
		if ( fTime < mControlPoints[index].mTime )
			break;

	// interpolate
	const ControlPoint &cp0 = mControlPoints[index-1];
	const ControlPoint &cp1 = mControlPoints[index];
	float deltaTime = cp1.mTime - cp0.mTime;
	float b = (fTime - cp0.mTime)/deltaTime;
	float a = 1.0f - b;

	pos = a*cp0.mPos + b*cp1.mPos + ((a*a*a-a)*cp0.mVel + (b*b*b-b)*cp1.mVel)*(deltaTime*deltaTime)/6.0f;
}

//*****************************************************************************
void VuCubicPosCurve::interpolate(int index0, int index1, float fRatio, VuVector3 &pos) const
{
	// interpolate
	const ControlPoint &cp0 = mControlPoints[index0];
	const ControlPoint &cp1 = mControlPoints[index1];
	float deltaTime = cp1.mTime - cp0.mTime;
	float b = fRatio;
	float a = 1.0f - b;

	pos = a*cp0.mPos + b*cp1.mPos + ((a*a*a-a)*cp0.mVel + (b*b*b-b)*cp1.mVel)*(deltaTime*deltaTime)/6.0f;
}

//*****************************************************************************
int VuCubicPosCurve::compareControlPoints(const void *p1, const void *p2)
{
	const ControlPoint *pCP1 = (const ControlPoint *)p1;
	const ControlPoint *pCP2 = (const ControlPoint *)p2;

	if ( pCP1->mTime < pCP2->mTime ) return -1;
	if ( pCP1->mTime > pCP2->mTime ) return 1;

	return 0;
}


//*****************************************************************************
void VuCubicPosCurve::spline(float *x, float *y, int n, float yp1, float ypn, float *y2, float *u)
{
	int i, k;
	float p, qn, sig, un;

	if ( yp1 > 0.99e30 )
	{
		y2[0]=0;
		u[0]=0;
	}
	else
	{
		y2[0]=-0.5f;
		u[0]=(3.0f/(x[1]-x[0]))*((y[1]-y[0])/(x[1]-x[0])-yp1);
	}

	for (i=1;i<=n-2;i++)
	{
		sig = (x[i]-x[i-1])/(x[i+1]-x[i-1]);
		p=sig*y2[i-1]+2.0f;
		y2[i]=(sig-1.0f)/p;
		u[i]=(y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
		u[i]=(6.0f*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
	}

	if ( ypn > 0.99e30 )
	{
		qn=0;
		un=0;
	}
	else
	{
		qn=0.5f;
		un=(3.0f/(x[n-1]-x[n-2]))*(ypn-(y[n-1]-y[n-2])/(x[n-1]-x[n-2]));
	}

	y2[n-1]=(un-qn*u[n-2])/(qn*y2[n-2]+1.0f);
	for (k=n-2;k>=0;k--)
		y2[k]=y2[k]*y2[k+1]+u[k];
}


//*****************************************************************************
VuCubicRotCurve::VuCubicRotCurve():
	mControlPoints(0),
	mIsBuilt(false)
{
}

//*****************************************************************************
void VuCubicRotCurve::clear()
{
	mControlPoints.clear();
	mIsBuilt = false;
}

//*****************************************************************************
void VuCubicRotCurve::reserve(int count)
{
	mControlPoints.reserve(count);
}

//*****************************************************************************
void VuCubicRotCurve::addControlPoint(const VuQuaternion &rot, float fTime)
{
	mControlPoints.resize(mControlPoints.size() + 1);
	mControlPoints.back().mRot = rot;
	mControlPoints.back().mTime = fTime;
}

//*****************************************************************************
void VuCubicRotCurve::sort()
{
	qsort(&mControlPoints[0], mControlPoints.size(), sizeof(mControlPoints[0]), compareControlPoints);
}

//*****************************************************************************
bool VuCubicRotCurve::build()
{
	int count = mControlPoints.size();

	// verify
	if ( count < 2 )
		return false;

	// start condition
	{
		VuQuaternion qRot = mControlPoints[0].mRot;
		VuQuaternion qRotNext = mControlPoints[1].mRot;
		mControlPoints[0].mQuad = VuMathUtil::splineQuaternion(qRotNext, qRot, qRotNext);
	}

	// end condition
	{
		VuQuaternion qRotPrev = mControlPoints[mControlPoints.size() - 2].mRot;
		VuQuaternion qRot = mControlPoints[mControlPoints.size() - 1].mRot;
		mControlPoints[mControlPoints.size() - 1].mQuad = VuMathUtil::splineQuaternion(qRotPrev, qRot, qRotPrev);
	}

	// other points
	for ( int i = 1; i < mControlPoints.size() - 1; i++ )
	{
		const VuQuaternion &qRotPrev = mControlPoints[i - 1].mRot;
		const VuQuaternion &qRot = mControlPoints[i].mRot;
		const VuQuaternion &qRotNext = mControlPoints[i + 1].mRot;
		mControlPoints[i].mQuad = VuMathUtil::splineQuaternion(qRotPrev, qRot, qRotNext);
	}

	mIsBuilt = true;

	return true;
}

//*****************************************************************************
void VuCubicRotCurve::getPointAtTime(float fTime, VuQuaternion &rot) const
{
	if ( fTime <= mControlPoints.begin().mTime )
	{
		rot = mControlPoints.begin().mRot;
		return;
	}

	if ( fTime >= mControlPoints.back().mTime )
	{
		rot = mControlPoints.back().mRot;
		return;
	}

	// find index
	int index;
	for ( index = 1; index < mControlPoints.size(); index++ )
		if ( fTime < mControlPoints[index].mTime )
			break;


	// interpolate
	const ControlPoint &cp0 = mControlPoints[index-1];
	const ControlPoint &cp1 = mControlPoints[index];
	float deltaTime = cp1.mTime - cp0.mTime;
	float ratio = (fTime - cp0.mTime)/deltaTime;

	rot = VuSquad(cp0.mRot, cp1.mRot, cp0.mQuad, cp1.mQuad, ratio);
}

//*****************************************************************************
void VuCubicRotCurve::interpolate(int index0, int index1, float fRatio, VuQuaternion &rot) const
{
	// interpolate
	const ControlPoint &cp0 = mControlPoints[index0];
	const ControlPoint &cp1 = mControlPoints[index1];

	rot = VuSquad(cp0.mRot, cp1.mRot, cp0.mQuad, cp1.mQuad, fRatio);
}

//*****************************************************************************
int VuCubicRotCurve::compareControlPoints(const void *p1, const void *p2)
{
	const ControlPoint *pCP1 = (const ControlPoint *)p1;
	const ControlPoint *pCP2 = (const ControlPoint *)p2;

	if ( pCP1->mTime < pCP2->mTime ) return -1;
	if ( pCP1->mTime > pCP2->mTime ) return 1;

	return 0;
}
