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

#include "VuCubicSpline.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Math/VuVector3.h"

//*****************************************************************************
template<typename T>
VuCubicSpline<T>::VuCubicSpline()
{

}

//*****************************************************************************
template<typename T>
VuCubicSpline<T>::~VuCubicSpline()
{

}

//*****************************************************************************
template<typename T>
void VuCubicSpline<T>::addControlPoint(const T &point)
{
	mControlPoints.push_back(point);
}

//*****************************************************************************
template<typename T>
bool VuCubicSpline<T>::build()
{
	mCubics.clear();
	mSegmentLengths.clear();
	mArcLength = 0.0f;

	if(mControlPoints.size() < 3)
	{
		return false;
	}

	VuArray<T> gamma;
	VuArray<T> delta;
	VuArray<T> D;

	int n = mControlPoints.size() - 1;
	gamma.resize(n + 1);
	delta.resize(n + 1);
	D.resize(n + 1);

	gamma[0] = T(1.0f / 2.0f);

	for(int i = 1; i < n; i++)
	{
		gamma[i] = T(1.0f) / (T(4.0f) - gamma[i - 1]);
	}

	gamma[n] = T(1.0f) / (T(2.0f) - gamma[n - 1]);

	//

	delta[0] = 3.0f * (mControlPoints[1] - mControlPoints[0]) * gamma[0];

	for(int i = 1; i < n; i++)
	{
		delta[i] = (3.0f * (mControlPoints[i + 1] - mControlPoints[i - 1]) - delta[i-1]) * gamma[i];
	}

	delta[n] = 3.0f * (mControlPoints[n] - mControlPoints[n-1]) * gamma[n];

	//

	D[n] = delta[n];

	for(int i = n-1; i >= 0; i--)
	{
		D[i] = delta[i] - gamma[i] * D[i+1];
	}

	for(int i = 0; i < n; i++)
	{
		const T &controlPointi = mControlPoints[i];
		const T &controlPointi_1 = mControlPoints[i+1];

		T a = controlPointi;
		T b = D[i];
		T c = 3.0f * (controlPointi_1 - controlPointi) - (2.0f * D[i]) - D[i+1];
		T d = 2.0f * (controlPointi - controlPointi_1) + D[i] + D[i+1];

		mCubics.push_back(VuCubic<T>(a, b, c, d));

		mSegmentLengths.push_back((controlPointi_1 - controlPointi).mag());
		mArcLength += mSegmentLengths.back();
	}

	return true;
}

template<typename T>
void VuCubicSpline<T>::clear()
{
	mControlPoints.clear();
	mCubics.clear();
	mSegmentLengths.clear();
	mArcLength = 0.0f;
}

//*****************************************************************************
template<typename T>
int VuCubicSpline<T>::getControlPointCount() const
{
	return mControlPoints.size();
}

//*****************************************************************************
template<typename T>
void VuCubicSpline<T>::getControlPoint(int index, T &point) const
{
	VUASSERT(index >= 0, "Cubic spline control point index out of bounds");
	VUASSERT(index < mControlPoints.size(), "Cubic spline control point index out of bounds");

	point = mControlPoints[index];
}

//*****************************************************************************
template<typename T>
void VuCubicSpline<T>::getPointOnSpline(float t, T &point) const
{
	VUASSERT(mControlPoints.size() >= 3, "Not enough control points in spline");

	if(t <= 0.0f) 
	{
		point = mControlPoints.front();
	}
	else if(t >= 1.0f)
	{
		point = mControlPoints.back();
	}
	else
	{
		float length = t * mArcLength;
		int index = 0;

		for(index = 0; index < mSegmentLengths.size(); index++)
		{
			if(length - mSegmentLengths[index] < 0.0f)
			{
				break;
			}

			length -= mSegmentLengths[index];
		}

		VUASSERT(index < mSegmentLengths.size(), "segmentlength index out of bounds");

		float u = length / mSegmentLengths[index];

		VUASSERT(u >= 0.0f, "");
		VUASSERT(u <= 1.0f, "");

		point = mCubics[index].eval(u);
	}
}

template<typename T>
float VuCubicSpline<T>::getArcLength() const
{
	return mArcLength;
}