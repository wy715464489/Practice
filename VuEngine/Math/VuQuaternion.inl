//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Quaternion inline functionality
// 
//*****************************************************************************

#include <float.h>
#include "VuVector3.h"


//*****************************************************************************
void VuQuaternion::loadIdentity()
{
	mVec.mX = 0; mVec.mY = 0; mVec.mZ = 0; mVec.mW = 1;
}

//*****************************************************************************
float VuQuaternion::normalize()
{
	float length = mVec.mag();

	if ( length > FLT_EPSILON )
	{
		mVec /= length;
	}
	else
	{
		length = 0;
		mVec = VuVector4(0,0,0,0);
	}

	return length;
}

//*****************************************************************************
void VuQuaternion::invert()
{
	float norm = VuDot(mVec, mVec);
	if ( norm > 0.0f )
	{
		float invNorm = 1.0f/norm;
		mVec.mX *= -invNorm;
		mVec.mY *= -invNorm;
		mVec.mZ *= -invNorm;
		mVec.mW *= invNorm;
	}
	else
	{
		mVec = VuVector4(0,0,0,0);
	}
}

//*****************************************************************************
VuQuaternion VuQuaternion::log() const
{
	float a = VuCos(mVec.mW);
	float sina = VuSin(a);

	VuQuaternion qRet;
	qRet.mVec.mW = 0;

	if ( sina > 0 )
	{
		float invsina = 1.0f/sina;
		qRet.mVec.mX = a*mVec.mX*invsina;
		qRet.mVec.mY = a*mVec.mY*invsina;
		qRet.mVec.mZ = a*mVec.mZ*invsina;
	}
	else
	{
		qRet.mVec.mX = 0;
		qRet.mVec.mY = 0;
		qRet.mVec.mZ = 0;
	}

	return qRet;
}

//*****************************************************************************
VuQuaternion VuQuaternion::exp() const
{
	float a = mVec.mag3d();
	float sina, cosa;
	VuSinCos(a, sina, cosa);

	VuQuaternion qRet;
	qRet.mVec.mW = cosa;

	if ( a > 0 )
	{
		float inva = 1.0f/a;
		qRet.mVec.mX = sina*mVec.mX*inva;
		qRet.mVec.mY = sina*mVec.mY*inva;
		qRet.mVec.mZ = sina*mVec.mZ*inva;
	}
	else
	{
		qRet.mVec.mX = 0;
		qRet.mVec.mY = 0;
		qRet.mVec.mZ = 0;
	}

	return qRet;
}

//*****************************************************************************
VuQuaternion VuQuaternion::conjugate() const
{
	return VuQuaternion(-mVec.mX, -mVec.mY, -mVec.mZ, mVec.mW);
}

//*****************************************************************************
void VuQuaternion::operator *=(const VuQuaternion &q)
{
	mVec.mX = q.mVec.mW*mVec.mX + q.mVec.mX*mVec.mW + q.mVec.mY*mVec.mZ - q.mVec.mZ*mVec.mY;
	mVec.mY = q.mVec.mW*mVec.mY + q.mVec.mY*mVec.mW + q.mVec.mZ*mVec.mX - q.mVec.mX*mVec.mZ;
	mVec.mZ = q.mVec.mW*mVec.mZ + q.mVec.mZ*mVec.mW + q.mVec.mX*mVec.mY - q.mVec.mY*mVec.mX;
	mVec.mW = q.mVec.mW*mVec.mW - q.mVec.mX*mVec.mX - q.mVec.mY*mVec.mY - q.mVec.mZ*mVec.mZ;
}

//*****************************************************************************
VuQuaternion operator *(const VuQuaternion &q1, const VuQuaternion &q2)
{
	VuQuaternion res;

	res.mVec.mX = q2.mVec.mW*q1.mVec.mX + q2.mVec.mX*q1.mVec.mW + q2.mVec.mY*q1.mVec.mZ - q2.mVec.mZ*q1.mVec.mY;
	res.mVec.mY = q2.mVec.mW*q1.mVec.mY + q2.mVec.mY*q1.mVec.mW + q2.mVec.mZ*q1.mVec.mX - q2.mVec.mX*q1.mVec.mZ;
	res.mVec.mZ = q2.mVec.mW*q1.mVec.mZ + q2.mVec.mZ*q1.mVec.mW + q2.mVec.mX*q1.mVec.mY - q2.mVec.mY*q1.mVec.mX;
	res.mVec.mW = q2.mVec.mW*q1.mVec.mW - q2.mVec.mX*q1.mVec.mX - q2.mVec.mY*q1.mVec.mY - q2.mVec.mZ*q1.mVec.mZ;

	return res;
}

//*****************************************************************************
VuQuaternion operator /(const VuQuaternion &q1, const VuQuaternion &q2)
{
	VuQuaternion res;

	const VuVector4 &v1 = q1.mVec;
	const VuVector4 &v2 = q2.mVec;
	VuVector4 &r = res.mVec;

	r.mW = v2.mW*v1.mW + v2.mX*v1.mX + v2.mY*v1.mY + v2.mZ*v1.mZ;
	r.mX = v2.mW*v1.mX - v2.mX*v1.mW - v2.mY*v1.mZ + v2.mZ*v1.mY;
	r.mY = v2.mW*v1.mY + v2.mX*v1.mZ - v2.mY*v1.mW - v2.mZ*v1.mX;
	r.mZ = v2.mW*v1.mZ - v2.mX*v1.mY + v2.mY*v1.mX - v2.mZ*v1.mW;

	res /= q2.mVec.magSquared();

	return res;
}
