//*****************************************************************************
//
//  Copyright (c) 2005-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Vector2d class
// 
//*****************************************************************************

#pragma once

#include "VuMath.h"


class VuVector2
{
public:
	VuVector2()								{}
	VuVector2(float x, float y)				{ mX = x; mY = y; }
	VuVector2(int x, int y)					{ mX = (float)x; mY = (float)y; }
	explicit VuVector2(const float *p)		{ mX = p[0]; mY = p[1]; }
	explicit VuVector2(float s)				{ mX = mY = s; }

	inline float	getValue(int i) const			{ return *(&mX + i); }
	inline void		setValue(int i, float value)	{ *(&mX + i) = value; }
	inline void		set(float x, float y)			{ mX = x; mY = y; }

	void operator *=(const VuVector2 &v)	{ mX *= v.mX; mY *= v.mY; }
	void operator /=(const VuVector2 &v)	{ mX /= v.mX; mY /= v.mY; }
	void operator +=(const VuVector2 &v)	{ mX += v.mX; mY += v.mY; }
	void operator -=(const VuVector2 &v)	{ mX -= v.mX; mY -= v.mY; }

	void operator *=(float f)				{ mX *= f; mY *= f; }
	void operator /=(float f)				{ float inv = 1.0f/f; mX *= inv; mY *= inv; }

	bool operator ==(const VuVector2 &v) const	{ return mX == v.mX && mY == v.mY; }
	bool operator !=(const VuVector2 &v) const	{ return mX != v.mX || mY != v.mY; }

	inline float magSquared() const			{ return mX*mX + mY*mY; }
	inline float mag() const				{ return VuSqrt(magSquared()); }
	inline VuVector2 normal() const;
	inline VuVector2 safeNormal() const;
	inline float normalize();
	inline float safeNormalize();

	float mX;
	float mY;

	static inline const VuVector2	&zero()	{ return smZeroVector2; }
	static inline const VuVector2	&one()	{ return smOneVector2; }

private:
	static VuVector2	smZeroVector2;
	static VuVector2	smOneVector2;
};


inline VuVector2 operator *(float f, const VuVector2 &v) { return VuVector2(v.mX*f, v.mY*f); }
inline VuVector2 operator *(const VuVector2 &v, float f) { return VuVector2(v.mX*f, v.mY*f); }
inline VuVector2 operator /(const VuVector2 &v, float f) { return VuVector2(v.mX/f, v.mY/f); }

inline VuVector2 operator *(int i, const VuVector2 &v) { return VuVector2(v.mX*i, v.mY*i); }
inline VuVector2 operator *(const VuVector2 &v, int i) { return VuVector2(v.mX*i, v.mY*i); }
inline VuVector2 operator /(const VuVector2 &v, int i) { return VuVector2(v.mX/i, v.mY/i); }

inline VuVector2 operator *(const VuVector2 &v1, const VuVector2 &v2) { return VuVector2(v1.mX*v2.mX, v1.mY*v2.mY); }
inline VuVector2 operator /(const VuVector2 &v1, const VuVector2 &v2) { return VuVector2(v1.mX/v2.mX, v1.mY/v2.mY); }
inline VuVector2 operator +(const VuVector2 &v1, const VuVector2 &v2) { return VuVector2(v1.mX+v2.mX, v1.mY+v2.mY); }
inline VuVector2 operator -(const VuVector2 &v1, const VuVector2 &v2) { return VuVector2(v1.mX-v2.mX, v1.mY-v2.mY); }

inline VuVector2 operator -(const VuVector2 &v) { return VuVector2(-v.mX, -v.mY); }

inline float VuDot(const VuVector2 &a, const VuVector2 &b)	{ return a.mX*b.mX + a.mY*b.mY; }

inline float VuDistSquared(const VuVector2 &a, const VuVector2 &b)		{ return (a.mX - b.mX)*(a.mX - b.mX) + (a.mY - b.mY)*(a.mY - b.mY); }
inline float VuDist(const VuVector2 &a, const VuVector2 &b)				{ return VuSqrt(VuDistSquared(a, b)); }

inline VuVector2 VuMin(const VuVector2 &a, const VuVector2 &b)
{
	return VuVector2(VuMin(a.mX, b.mX), VuMin(a.mY, b.mY));
}

inline VuVector2 VuMax(const VuVector2 &a, const VuVector2 &b)
{
	return VuVector2(VuMax(a.mX, b.mX), VuMax(a.mY, b.mY));
}

inline void VuMinMax(const VuVector2 &v, VuVector2 &vMin, VuVector2 &vMax)
{
	vMin = VuVector2(VuMin(v.mX, vMin.mX), VuMin(v.mY, vMin.mY));
	vMax = VuVector2(VuMax(v.mX, vMax.mX), VuMax(v.mY, vMax.mY));
}

inline VuVector2 VuLerp(const VuVector2 &a, const VuVector2 &b, float factor)
{
	return (1.0f - factor)*a + factor*b;
}

inline VuVector2 VuVector2::normal() const
{
	return (*this)/mag();
}

inline VuVector2 VuVector2::safeNormal() const
{
	float fMag = mag();
	fMag = VuSelect(fMag - FLT_EPSILON, fMag, 1.0f); 
	return (*this)/fMag;
}

inline float VuVector2::normalize()
{
	float fMag = mag();
	*this /= fMag;
	return fMag;
}

inline float VuVector2::safeNormalize()
{
	float fMag = mag();
	fMag = VuSelect(fMag - FLT_EPSILON, fMag, 1.0f); 
	*this /= fMag;
	return fMag;
}
