//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Quaternion class
// 
//*****************************************************************************

#pragma once

#include "VuMath.h"
#include "VuVector4.h"

class VuVector3;
class VuMatrix;


class VuQuaternion
{
public:

	// construction
	inline VuQuaternion()									{}
	inline VuQuaternion(float x, float y, float z, float r)	{ mVec.mX = x; mVec.mY = y; mVec.mZ = z; mVec.mW = r; }
	inline VuQuaternion(const VuVector4 &v)					{ mVec = v; }
	explicit inline VuQuaternion(const VuMatrix &mat)		{ fromRotationMatrix(mat); }
	inline VuQuaternion(const VuVector3 &axis, float angle)	{ fromAxisAngle(axis, angle); }

    // conversion
	void				fromRotationMatrix(const VuMatrix &mat);
	void				toRotationMatrix(VuMatrix &mat) const;
	void				fromAxisAngle(const VuVector3 &axis, float angle);
	void				toAxisAngle(VuVector3 &axis, float &angle) const;
	void				fromEulerAngles(const VuVector3 &euler);
	void				toEulerAngles(VuVector3 &euler) const;

	// operations
	inline void			loadIdentity();
	inline float		normalize();
	inline void			invert();
	inline VuQuaternion	log() const;
	inline VuQuaternion	exp() const;
	inline VuQuaternion	conjugate() const;

	// arithmetic
	inline void operator *=(float f)				{ mVec *= f; }
	inline void operator /=(float f)				{ mVec /= f; }
	inline void operator +=(const VuQuaternion &q)	{ mVec += q.mVec; }
	inline void operator -=(const VuQuaternion &q)	{ mVec -= q.mVec; }
	inline void operator *=(const VuQuaternion &q);

	static inline const VuQuaternion	&identity()	{ return smIdentityQuaternion; }

	VuVector4	mVec;

private:
	static VuQuaternion	smIdentityQuaternion;
};


inline VuQuaternion operator *(float f, const VuQuaternion &q)	{ return VuQuaternion(f*q.mVec); }
inline VuQuaternion operator *(const VuQuaternion &q, float f)	{ return VuQuaternion(q.mVec*f); }
inline VuQuaternion operator /(const VuQuaternion &q, float f)	{ return VuQuaternion(q.mVec/f); }

inline VuQuaternion operator +(const VuQuaternion &q1, const VuQuaternion &q2)	{ return VuQuaternion(q1.mVec+q2.mVec); }
inline VuQuaternion operator -(const VuQuaternion &q1, const VuQuaternion &q2)	{ return VuQuaternion(q1.mVec-q2.mVec); }
inline VuQuaternion operator *(const VuQuaternion &q1, const VuQuaternion &q2);
inline VuQuaternion operator /(const VuQuaternion &q1, const VuQuaternion &q2);

VuQuaternion VuSlerp(const VuQuaternion &q0, const VuQuaternion &q1, float t);
VuQuaternion VuSlerpNoInvert(const VuQuaternion &q0, const VuQuaternion &q1, float t);
VuQuaternion VuSquad(const VuQuaternion &q0, const VuQuaternion &q1, const VuQuaternion &a, const VuQuaternion &b, float t);
VuVector3 VuSlerp(const VuVector3 &rot0, const VuVector3 &rot1, float t);


#include "VuQuaternion.inl"

