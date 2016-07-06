//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Cubic Curve class
//
//  [Numerical Recipes in C, the Art of Scientific Computing (2nd Edition)]
//
//  The difference between this class and the VuCubicSpline class is that
//  the user is able to specify the time for each control point.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuQuaternion.h"


//*****************************************************************************
class VuCubicPosCurve
{
public:
	VuCubicPosCurve();

	void 		clear();
	void		reserve(int count);
	void		addControlPoint(const VuVector3 &pos, float fTime);
	void		sort();
	bool		build(const VuVector3 &velStart, const VuVector3 &velEnd);
	bool		isBuilt() const	{ return mIsBuilt; }

	int			getControlPointCount() const	{ return mControlPoints.size(); }
	float		getTotalTime() const			{ return mControlPoints.back().mTime; }
	void		getPointAtTime(float fTime, VuVector3 &pos) const;

	// use with caution... indices are not checked
	void		interpolate(int index0, int index1, float fRatio, VuVector3 &pos) const;

protected:
	struct ControlPoint
	{
		VuVector3	mPos;
		VuVector3	mVel;
		float		mTime;
	};
	typedef VuArray<ControlPoint> ControlPoints;

	static int		compareControlPoints(const void *p1, const void *p2);
	void			spline(float *x, float *y, int n, float yp1, float ypn, float *y2, float *u);

	ControlPoints	mControlPoints;
	bool			mIsBuilt;
};


//*****************************************************************************
class VuCubicRotCurve
{
public:
	VuCubicRotCurve();

	void 		clear();
	void		reserve(int count);
	void		addControlPoint(const VuQuaternion &rot, float fTime);
	void		sort();
	bool		build();
	bool		isBuilt() const	{ return mIsBuilt; }

	int			getControlPointCount() const	{ return mControlPoints.size(); }
	float		getTotalTime() const			{ return mControlPoints.back().mTime; }
	void		getPointAtTime(float fTime, VuQuaternion &rot) const;

	// use with caution... indices are not checked
	void		interpolate(int index0, int index1, float fRatio, VuQuaternion &rot) const;

protected:
	struct ControlPoint
	{
		VuQuaternion	mRot;
		VuQuaternion	mQuad;
		float			mTime;
	};
	typedef VuArray<ControlPoint> ControlPoints;

	static int		compareControlPoints(const void *p1, const void *p2);

	ControlPoints	mControlPoints;
	bool			mIsBuilt;
};
