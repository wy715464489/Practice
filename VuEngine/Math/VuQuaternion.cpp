//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Quaternion class
// 
//*****************************************************************************

#include "VuQuaternion.h"
#include "VuMatrix.h"


VuQuaternion VuQuaternion::smIdentityQuaternion(0, 0, 0, 1);


//*****************************************************************************
void VuQuaternion::fromRotationMatrix(const VuMatrix &mat)
{
	float trace = mat.get(0,0) + mat.get(1,1) + mat.get(2,2);

	if ( trace > 0 )
	{
		float root = VuSqrt(trace + 1.0f);
		mVec.mW = 0.5f*root;
		root = 0.5f/root;
		mVec.mX = (mat.get(2,1) - mat.get(1,2))*root;
		mVec.mY = (mat.get(0,2) - mat.get(2,0))*root;
		mVec.mZ = (mat.get(1,0) - mat.get(0,1))*root;
	}
	else
	{
		const int next[3] = {1,2,0};
		int i = 0;
		if ( mat.get(1,1) > mat.get(0,0) ) i = 1;
		if ( mat.get(2,2) > mat.get(i,i) ) i = 2;
		const int j = next[i];
		const int k = next[j];

		float root = VuSqrt(mat.get(i,i) - mat.get(j,j) - mat.get(k,k) + 1.0f);
		mVec.setValue(i, root*0.5f);
		root = 0.5f/root;
		mVec.setValue(3, (mat.get(k,j) - mat.get(j,k))*root);
		mVec.setValue(j, (mat.get(j,i) + mat.get(i,j))*root);
		mVec.setValue(k, (mat.get(k,i) + mat.get(i,k))*root);
	}
}

//*****************************************************************************
void VuQuaternion::toRotationMatrix(VuMatrix &mat) const
{
	float x = 2.0f*mVec.mX;
	float y = 2.0f*mVec.mY;
	float z = 2.0f*mVec.mZ;
	float wx = mVec.mW*x;
	float wy = mVec.mW*y;
	float wz = mVec.mW*z;
	float xx = mVec.mX*x;
	float xy = mVec.mX*y;
	float xz = mVec.mX*z;
	float yy = mVec.mY*y;
	float yz = mVec.mY*z;
	float zz = mVec.mZ*z;

	mat.mX = VuVector4(1.0f - (yy + zz), xy - wz, xz + wy, 0);
	mat.mY = VuVector4(xy + wz, 1.0f - (xx + zz), yz - wx, 0);
	mat.mZ = VuVector4(xz - wy, yz + wx, 1.0f - (xx + yy), 0);
	mat.mT = VuVector4(0, 0, 0, 1);
}

//*****************************************************************************
// assume axis is unit length
//
void VuQuaternion::fromAxisAngle(const VuVector3 &axis, float angle)
{
	float s, c;
	VuSinCos(0.5f*angle, s, c);

	mVec.mX = s*axis.mX;
	mVec.mY = s*axis.mY;
	mVec.mZ = s*axis.mZ;
	mVec.mW = c;
}

//*****************************************************************************
void VuQuaternion::toAxisAngle(VuVector3 &axis, float &angle) const
{
	float sqrLength = mVec.mag3dSquared();
	if ( sqrLength > FLT_EPSILON )
	{
		axis = VuVector3(mVec.mX, mVec.mY, mVec.mZ);
		axis.normalize();
		if ( mVec.mW < 0.0f )
			axis = -axis;

		angle = 2.0f*VuACos(VuMin(VuAbs(mVec.mW), 1.0f));
	}
	else
	{
		axis = VuVector3(0,0,1);
		angle = 0;
	}
}

//*****************************************************************************
void VuQuaternion::fromEulerAngles(const VuVector3 &euler)
{
	VuMatrix mat;
	mat.setEulerAngles(euler);
	fromRotationMatrix(mat);
}

//*****************************************************************************
void VuQuaternion::toEulerAngles(VuVector3 &euler) const
{
	VuMatrix mat;
	toRotationMatrix(mat);
	euler = mat.getEulerAngles();
}

//*****************************************************************************
VuQuaternion VuSlerp(const VuQuaternion &q0, const VuQuaternion &q1, float t)
{
	VuQuaternion qr;
	VuQuaternion q00;
	float d = VuDot(q0.mVec, q1.mVec);

	// Check the sign of the dot product in order to take the shortest route 
	// between the two orientations
	if(d < 0.0f)
	{
		d = -d;
		q00 = -q0.mVec;
	}
	else
	{
		q00 = q0;
	}

	d = VuMin(d, 1.0f);
	float angle = VuACos(d);

	if ( angle > FLT_EPSILON )
	{
		float invSin = 1.0f/VuSin(angle);
		float coeff0 = VuSin(angle - t*angle)*invSin;
		float coeff1 = VuSin(t*angle)*invSin;
		qr = coeff0*q00 + coeff1*q1;
	}
	else
	{
		qr = q0;
	}

	return qr;
}

//*****************************************************************************
VuQuaternion VuSlerpNoInvert(const VuQuaternion &q0, const VuQuaternion &q1, float t)
{
	VuQuaternion qr;
	float d = VuDot(q0.mVec, q1.mVec);

	d = VuMin(d, 1.0f);
	float angle = VuACos(d);

	if ( angle > FLT_EPSILON && angle < VU_PI - FLT_EPSILON )
	{
		float invSin = 1.0f/VuSin(angle);
		float coeff0 = VuSin(angle - t*angle)*invSin;
		float coeff1 = VuSin(t*angle)*invSin;
		qr = coeff0*q0 + coeff1*q1;
	}
	else
	{
		qr = q0;
	}

	return qr;
}

//*****************************************************************************
VuQuaternion VuSquad(const VuQuaternion &q0, const VuQuaternion &q1, const VuQuaternion &a, const VuQuaternion &b, float t)
{
	return VuSlerp(VuSlerpNoInvert(q0, q1, t), VuSlerpNoInvert(a, b, t), 2*t*(1-t));
}

//*****************************************************************************
VuVector3 VuSlerp(const VuVector3 &rot0, const VuVector3 &rot1, float t)
{
	VuMatrix mat0, mat1;
	mat0.setEulerAngles(rot0);
	mat1.setEulerAngles(rot1);
	VuQuaternion q0(mat0);
	VuQuaternion q1(mat1);

	VuQuaternion q = VuSlerp(q0, q1, t);
	VuMatrix mat;
	q.toRotationMatrix(mat);

	return mat.getEulerAngles();
}