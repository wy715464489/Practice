//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Matrix class
// 
//*****************************************************************************

#pragma once

#include "VuVector2.h"
#include "VuVector3.h"
#include "VuVector4.h"

class VuMatrix
{
public:
	VuMatrix()																						{}
	VuMatrix(const VuVector4 &vX, const VuVector4 &vY, const VuVector4 &vZ, const VuVector4 &vT)	{mX = vX; mY = vY; mZ = vZ; mT = vT; }

	inline void				setAxis(int i, const VuVector3 &v)	{ (&mX + i)->mX = v.mX; (&mX + i)->mY = v.mY; (&mX + i)->mZ = v.mZ; }
	inline void				setAxisX(const VuVector3 &v)		{ mX.mX = v.mX; mX.mY = v.mY; mX.mZ = v.mZ; }
	inline void				setAxisY(const VuVector3 &v)		{ mY.mX = v.mX; mY.mY = v.mY; mY.mZ = v.mZ; }
	inline void				setAxisZ(const VuVector3 &v)		{ mZ.mX = v.mX; mZ.mY = v.mY; mZ.mZ = v.mZ; }
	inline void				setTrans(const VuVector3 &v)		{ mT.mX = v.mX; mT.mY = v.mY; mT.mZ = v.mZ; }

	inline const VuVector3	&getAxis(int i) const	{ return (VuVector3 &)*(&mX + i); }
	inline const VuVector3	&getAxisX() const		{ return (const VuVector3 &)mX; }
	inline const VuVector3	&getAxisY() const		{ return (const VuVector3 &)mY; }
	inline const VuVector3	&getAxisZ() const		{ return (const VuVector3 &)mZ; }
	inline const VuVector3	&getTrans() const		{ return (const VuVector3 &)mT; }

	inline float			get(int i, int j) const			{ return *(((float *)(&mX + i)) + j); }
	inline void				set(int i, int j, float value)	{ *(((float *)(&mX + i)) + j) = value; }

	VuVector3				getEulerAngles() const;
	void					setEulerAngles(const VuVector3 &v);

	inline void				loadIdentity();
	inline void				transpose();
	inline void				invert();
	inline void				fastInvert(); // assumes orthonormal rotation
	inline void				translate(const VuVector3 &v)					{ *this = (*this)*translation(v); }
	inline void				rotateX(float fAngle)							{ *this = (*this)*rotationX(fAngle); }
	inline void				rotateY(float fAngle)							{ *this = (*this)*rotationY(fAngle); }
	inline void				rotateZ(float fAngle)							{ *this = (*this)*rotationZ(fAngle); }
	inline void				rotateXYZ(const VuVector3 &v)					{ *this = (*this)*rotationXYZ(v); }
	inline void				rotateAxis(const VuVector3 &v, float angle)		{ *this = (*this)*rotationAxis(v, angle); }
	inline void				scale(const VuVector3 &v)						{ *this = (*this)*scaling(v); }

	inline void				translateLocal(const VuVector3 &v)					{ *this = translation(v)*(*this); }
	inline void				rotateXLocal(float fAngle)							{ *this = rotationX(fAngle)*(*this); }
	inline void				rotateYLocal(float fAngle)							{ *this = rotationY(fAngle)*(*this); }
	inline void				rotateZLocal(float fAngle)							{ *this = rotationZ(fAngle)*(*this); }
	inline void				rotateXYZLocal(const VuVector3 &v)					{ *this = rotationXYZ(v)*(*this); }
	inline void				rotateAxisLocal(const VuVector3 &v, float angle)	{ *this = rotationAxis(v, angle)*(*this); }
	inline void				scaleLocal(const VuVector3 &v)						{ *this = scaling(v)*(*this); }
	inline void				scaleLocal(float f)									{ *this = scaling(f)*(*this); }

	inline VuMatrix			operator *(const VuMatrix &mat) const;
	inline void				operator *=(const VuMatrix &mat);

	inline VuVector2		transform(const VuVector2 &v) const;

	inline VuVector3		transform(const VuVector3 &v) const;
	inline VuVector3		transformCoord(const VuVector3 &v) const;
	inline VuVector3		transformNormal(const VuVector3 &v) const;

	inline VuVector4		transform(const VuVector4 &v) const;

	inline float			determinant() const;

	VuVector4 mX;
	VuVector4 mY;
	VuVector4 mZ;
	VuVector4 mT;

	static inline const VuMatrix	&identity()	{ return smIdentityMatrix; }
	static inline VuMatrix			translation(const VuVector3 &v);
	static inline VuMatrix			rotationX(float fAngle);
	static inline VuMatrix			rotationY(float fAngle);
	static inline VuMatrix			rotationZ(float fAngle);
	static inline VuMatrix			rotationXYZ(const VuVector3 &v);
	static inline VuMatrix			rotationAxis(const VuVector3 &v, float angle);
	static inline VuMatrix			scaling(const VuVector3 &v);
	static inline VuMatrix			scaling(float f);

private:
	static VuMatrix	smIdentityMatrix;
};


#if defined (VUWIN32)
	#include "D3d11/VuD3d11Matrix.inl"
#elif defined (VUXBOX360)
	#include "D3d9/VuD3d9Matrix.inl"
#elif defined (VUWINSTORE) || defined (VUWINPHONE)
	#if defined (_M_ARM)
		#include "Generic/VuGenericMatrix.inl"
	#else
		#include "D3d11/VuD3d11Matrix.inl"
	#endif
#elif defined (VUXB1)
	#include "D3d11/VuD3d11Matrix.inl"
#else
	#include "Generic/VuGenericMatrix.inl"
#endif
