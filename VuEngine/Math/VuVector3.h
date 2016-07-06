//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Vector3d class
// 
//*****************************************************************************

#pragma once

#include "VuMath.h"

class VuVector3
{
public:
	VuVector3()								{}
	VuVector3(float x, float y, float z)	{ mX = x; mY = y; mZ = z; }
	explicit VuVector3(const float *p)		{ mX = p[0]; mY = p[1]; mZ = p[2]; }
	explicit VuVector3(float s)				{ mX = mY = mZ = s; }

	inline float	getValue(int i) const			{ return *(&mX + i); }
	inline void		setValue(int i, float value)	{ *(&mX + i) = value; }
	inline void		set(float x, float y, float z)	{ mX = x; mY = y; mZ = z; }

	void operator *=(const VuVector3 &v)	{ mX *= v.mX; mY *= v.mY; mZ *= v.mZ; }
	void operator /=(const VuVector3 &v)	{ mX /= v.mX; mY /= v.mY; mZ /= v.mZ; }
	void operator +=(const VuVector3 &v)	{ mX += v.mX; mY += v.mY; mZ += v.mZ; }
	void operator -=(const VuVector3 &v)	{ mX -= v.mX; mY -= v.mY; mZ -= v.mZ; }

	void operator *=(float f)				{ mX *= f; mY *= f; mZ *= f; }
	void operator /=(float f)				{ float inv = 1.0f/f; mX *= inv; mY *= inv; mZ *= inv; }

	bool operator ==(const VuVector3 &v) const	{ return mX == v.mX && mY == v.mY && mZ == v.mZ; }
	bool operator !=(const VuVector3 &v) const	{ return mX != v.mX || mY != v.mY || mZ != v.mZ; }

	inline float magSquared() const			{ return mX*mX + mY*mY + mZ*mZ; }
	inline float mag() const				{ return VuSqrt(magSquared()); }
	inline float mag2dSquared() const		{ return mX*mX + mY*mY; }
	inline float mag2d() const				{ return VuSqrt(mag2dSquared()); }
	inline VuVector3 normal() const;
	inline VuVector3 safeNormal() const;
	inline float normalize();
	inline float safeNormalize();

	float mX;
	float mY;
	float mZ;
	float mPad;

	static inline const VuVector3	&zero()	{ return smZeroVector3; }
	static inline const VuVector3	&one()	{ return smOneVector3; }

private:
	static VuVector3	smZeroVector3;
	static VuVector3	smOneVector3;
};


inline VuVector3 operator *(float f, const VuVector3 &v) { return VuVector3(v.mX*f, v.mY*f, v.mZ*f); }
inline VuVector3 operator *(const VuVector3 &v, float f) { return VuVector3(v.mX*f, v.mY*f, v.mZ*f); }
inline VuVector3 operator /(const VuVector3 &v, float f) { return VuVector3(v.mX/f, v.mY/f, v.mZ/f); }

inline VuVector3 operator *(const VuVector3 &v1, const VuVector3 &v2) { return VuVector3(v1.mX*v2.mX, v1.mY*v2.mY, v1.mZ*v2.mZ); }
inline VuVector3 operator /(const VuVector3 &v1, const VuVector3 &v2) { return VuVector3(v1.mX/v2.mX, v1.mY/v2.mY, v1.mZ/v2.mZ); }
inline VuVector3 operator +(const VuVector3 &v1, const VuVector3 &v2) { return VuVector3(v1.mX+v2.mX, v1.mY+v2.mY, v1.mZ+v2.mZ); }
inline VuVector3 operator -(const VuVector3 &v1, const VuVector3 &v2) { return VuVector3(v1.mX-v2.mX, v1.mY-v2.mY, v1.mZ-v2.mZ); }

inline VuVector3 operator -(const VuVector3 &v) { return VuVector3(-v.mX, -v.mY, -v.mZ); }

inline float VuDot(const VuVector3 &a, const VuVector3 &b)	{ return a.mX*b.mX + a.mY*b.mY + a.mZ*b.mZ; }

inline float VuDistSquared(const VuVector3 &a, const VuVector3 &b)		{ return (a.mX - b.mX)*(a.mX - b.mX) + (a.mY - b.mY)*(a.mY - b.mY) + (a.mZ - b.mZ)*(a.mZ - b.mZ); }
inline float VuDist(const VuVector3 &a, const VuVector3 &b)				{ return VuSqrt(VuDistSquared(a, b)); }
inline float VuDist2dSquared(const VuVector3 &a, const VuVector3 &b)	{ return (a.mX - b.mX)*(a.mX - b.mX) + (a.mY - b.mY)*(a.mY - b.mY); }
inline float VuDist2d(const VuVector3 &a, const VuVector3 &b)			{ return VuSqrt(VuDist2dSquared(a, b)); }

inline VuVector3 VuCross(const VuVector3 &a, const VuVector3 &b)
{
	return VuVector3( a.mY*b.mZ - b.mY*a.mZ, b.mX*a.mZ - a.mX*b.mZ, a.mX*b.mY - b.mX*a.mY );
}

inline VuVector3 VuMin(const VuVector3 &a, const VuVector3 &b)
{
	return VuVector3(VuMin(a.mX, b.mX), VuMin(a.mY, b.mY), VuMin(a.mZ, b.mZ));
}

inline VuVector3 VuMax(const VuVector3 &a, const VuVector3 &b)
{
	return VuVector3(VuMax(a.mX, b.mX), VuMax(a.mY, b.mY), VuMax(a.mZ, b.mZ));
}

inline void VuMinMax(const VuVector3 &v, VuVector3 &vMin, VuVector3 &vMax)
{
	vMin = VuVector3(VuMin(v.mX, vMin.mX), VuMin(v.mY, vMin.mY), VuMin(v.mZ, vMin.mZ));
	vMax = VuVector3(VuMax(v.mX, vMax.mX), VuMax(v.mY, vMax.mY), VuMax(v.mZ, vMax.mZ));
}

inline VuVector3 VuLerp(const VuVector3 &a, const VuVector3 &b, float factor)
{
	return (1.0f - factor)*a + factor*b;
}

inline VuVector3 VuRound(const VuVector3 &v, float precision)
{
	int x = VuRound(v.mX/precision);
	int y = VuRound(v.mY/precision);
	int z = VuRound(v.mZ/precision);
	return VuVector3(x*precision, y*precision, z*precision);
}

inline VuVector3 VuRadiansToDegrees(const VuVector3 &v)
{
	return VuVector3(VuRadiansToDegrees(v.mX), VuRadiansToDegrees(v.mY), VuRadiansToDegrees(v.mZ));
}	

inline VuVector3 VuDegreesToRadians(const VuVector3 &v)
{
	return VuVector3(VuDegreesToRadians(v.mX), VuDegreesToRadians(v.mY), VuDegreesToRadians(v.mZ));
}	

inline bool VuIsFinite(const VuVector3 &v)
{
	return VuIsFinite(v.mX) && VuIsFinite(v.mY) && VuIsFinite(v.mZ);
}

inline VuVector3 VuReflect(const VuVector3 &v, const VuVector3 &n)
{
	return v - 2.0f*n*VuDot(v, n);
}

inline VuVector3 VuVector3::normal() const
{
	return (*this)/mag();
}

inline VuVector3 VuVector3::safeNormal() const
{
	float fMag = mag();
	fMag = VuSelect(fMag - FLT_EPSILON, fMag, 1.0f); 
	return (*this)/fMag;
}

inline float VuVector3::normalize()
{
	float fMag = mag();
	*this /= fMag;
	return fMag;
}

inline float VuVector3::safeNormalize()
{
	float fMag = mag();
	fMag = VuSelect(fMag - FLT_EPSILON, fMag, 1.0f); 
	*this /= fMag;
	return fMag;
}
