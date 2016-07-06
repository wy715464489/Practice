//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Vector4d class
// 
//*****************************************************************************

#pragma once

#include "VuMath.h"

class VuVector4
{
public:
	VuVector4()										{}
	VuVector4(float x, float y, float z, float w)	{ mX = x; mY = y; mZ = z; mW = w; }
	explicit VuVector4(const float *p)				{ mX = p[0]; mY = p[1]; mZ = p[2]; mW = p[3]; }
	explicit VuVector4(float s)						{ mX = mY = mZ = mW = s; }

	inline float	getValue(int i) const					{ return *(&mX + i); }
	inline void		setValue(int i, float value)			{ *(&mX + i) = value; }
	inline void		set(float x, float y, float z, float w)	{ mX = x; mY = y; mZ = z; mW = w; }

	void operator *=(const VuVector4 &v)	{ mX *= v.mX; mY *= v.mY; mZ *= v.mZ; mW *= v.mW; }
	void operator /=(const VuVector4 &v)	{ mX /= v.mX; mY /= v.mY; mZ /= v.mZ; mW /= v.mW; }
	void operator +=(const VuVector4 &v)	{ mX += v.mX; mY += v.mY; mZ += v.mZ; mW += v.mW; }
	void operator -=(const VuVector4 &v)	{ mX -= v.mX; mY -= v.mY; mZ -= v.mZ; mW -= v.mW; }

	void operator *=(float f)				{ mX *= f; mY *= f; mZ *= f; mW *= f; }
	void operator /=(float f)				{ float inv = 1.0f/f; mX *= inv; mY *= inv; mZ *= inv; mW *= inv; }

	bool operator ==(const VuVector4 &v) const	{ return mX == v.mX && mY == v.mY && mZ == v.mZ && mW == v.mW; }
	bool operator !=(const VuVector4 &v) const	{ return mX != v.mX || mY != v.mY || mZ != v.mZ || mW != v.mW; }

	inline float magSquared() const			{ return mX*mX + mY*mY + mZ*mZ + mW*mW; }
	inline float mag() const				{ return VuSqrt(magSquared()); }
	inline float mag3dSquared() const		{ return mX*mX + mY*mY + mZ*mZ; }
	inline float mag3d() const				{ return VuSqrt(mag3dSquared()); }
	inline float mag2dSquared() const		{ return mX*mX + mY*mY; }
	inline float mag2d() const				{ return VuSqrt(mag2dSquared()); }

	float mX;
	float mY;
	float mZ;
	float mW;

	static inline const VuVector4	&zero()	{ return smZeroVector4; }
	static inline const VuVector4	&one()	{ return smOneVector4; }

private:
	static VuVector4	smZeroVector4;
	static VuVector4	smOneVector4;
};


inline VuVector4 operator *(float f, const VuVector4 &v) { return VuVector4(v.mX*f, v.mY*f, v.mZ*f, v.mW*f); }
inline VuVector4 operator *(const VuVector4 &v, float f) { return VuVector4(v.mX*f, v.mY*f, v.mZ*f, v.mW*f); }
inline VuVector4 operator /(const VuVector4 &v, float f) { return VuVector4(v.mX/f, v.mY/f, v.mZ/f, v.mW/f); }

inline VuVector4 operator *(const VuVector4 &v1, const VuVector4 &v2) { return VuVector4(v1.mX*v2.mX, v1.mY*v2.mY, v1.mZ*v2.mZ, v1.mW*v2.mW); }
inline VuVector4 operator /(const VuVector4 &v1, const VuVector4 &v2) { return VuVector4(v1.mX/v2.mX, v1.mY/v2.mY, v1.mZ/v2.mZ, v1.mW/v2.mW); }
inline VuVector4 operator +(const VuVector4 &v1, const VuVector4 &v2) { return VuVector4(v1.mX+v2.mX, v1.mY+v2.mY, v1.mZ+v2.mZ, v1.mW+v2.mW); }
inline VuVector4 operator -(const VuVector4 &v1, const VuVector4 &v2) { return VuVector4(v1.mX-v2.mX, v1.mY-v2.mY, v1.mZ-v2.mZ, v1.mW-v2.mW); }

inline VuVector4 operator -(const VuVector4 &v) { return VuVector4(-v.mX, -v.mY, -v.mZ, -v.mW); }

inline float VuDot(const VuVector4 &a, const VuVector4 &b) { return a.mX*b.mX + a.mY*b.mY + a.mZ*b.mZ + a.mW*b.mW; }

inline VuVector4 VuMin(const VuVector4 &a, const VuVector4 &b)
{
	return VuVector4(VuMin(a.mX, b.mX), VuMin(a.mY, b.mY), VuMin(a.mZ, b.mZ), VuMin(a.mW, b.mW));
}

inline VuVector4 VuMax(const VuVector4 &a, const VuVector4 &b)
{
	return VuVector4(VuMax(a.mX, b.mX), VuMax(a.mY, b.mY), VuMax(a.mZ, b.mZ), VuMax(a.mW, b.mW));
}

inline void VuMinMax(const VuVector4 &v, VuVector4 &vMin, VuVector4 &vMax)
{
	vMin = VuVector4(VuMin(v.mX, vMin.mX), VuMin(v.mY, vMin.mY), VuMin(v.mZ, vMin.mZ), VuMin(v.mW, vMin.mW));
	vMax = VuVector4(VuMax(v.mX, vMax.mX), VuMax(v.mY, vMax.mY), VuMax(v.mZ, vMax.mZ), VuMax(v.mW, vMax.mW));
}

inline VuVector4 VuLerp(const VuVector4 &a, const VuVector4 &b, float factor)
{
	return (1.0f - factor)*a + factor*b;
}
