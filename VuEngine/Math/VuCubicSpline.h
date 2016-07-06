//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Natural Cubic Spline Class
//
//	VuCubic code based on Tim Lambert
//
//	http://www.cse.unsw.edu.au/~lambert/
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"

class VuVector3;

//*****************************************************************************
template<typename T>
class VuCubic
{
public:

	VuCubic(const T &a, const T &b, const T &c, const T &d)
		: mA(a), mB(b), mC(c), mD(d)
	{

	}

	inline T eval(float u) const
	{
		// a + b*u + c*u^2 +d*u^3
		return (((mD * u) + mC) * u + mB) * u + mA;
	}

	inline T tangent(float u) const
	{
		// b + 2*c*u + 3*d*u^2
		return ((3 * mD * u) + 2 * mC) * u + mB;
	}

private:

	T mA;
	T mB;
	T mC;
	T mD;
};

//*****************************************************************************
template<typename T>
class VuCubicSpline
{
public:

	VuCubicSpline();
	~VuCubicSpline();

	inline void		addControlPoint(const T &point);
	inline bool		build();
	inline void 	clear();

	inline int		getControlPointCount() const;
	inline void		getControlPoint(int index, T &point) const;

	// t is a parametric value (0.0f - 1.0f)
	inline void		getPointOnSpline(float t, T &point) const;

	inline float	getArcLength() const;

protected:

	typedef VuArray<T> ControlPoints;
	typedef VuArray<VuCubic<T>> Cubics;
	typedef VuArray<float> Lengths;

	ControlPoints	mControlPoints;
	Cubics			mCubics;
	Lengths			mSegmentLengths;
	float			mArcLength;
};

#include "VuCubicSpline.inl"
