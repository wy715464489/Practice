//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Generic inline functions for Matrix class
// 
//*****************************************************************************

#include <float.h>


void VuMatrix::loadIdentity()
{
	mX.mX = 1.0f; mX.mY = 0.0f; mX.mZ = 0.0f; mX.mW = 0.0f;
	mY.mX = 0.0f; mY.mY = 1.0f; mY.mZ = 0.0f; mY.mW = 0.0f;
	mZ.mX = 0.0f; mZ.mY = 0.0f; mZ.mZ = 1.0f; mZ.mW = 0.0f;
	mT.mX = 0.0f; mT.mY = 0.0f; mT.mZ = 0.0f; mT.mW = 1.0f;
}

void VuMatrix::transpose()
{
	VuMatrix out;

	out.mX.mX = mX.mX; out.mX.mY = mY.mX; out.mX.mZ = mZ.mX; out.mX.mW = mT.mX;
	out.mY.mX = mX.mY; out.mY.mY = mY.mY; out.mY.mZ = mZ.mY; out.mY.mW = mT.mY;
	out.mZ.mX = mX.mZ; out.mZ.mY = mY.mZ; out.mZ.mZ = mZ.mZ; out.mZ.mW = mT.mZ;
	out.mT.mX = mX.mW; out.mT.mY = mY.mW; out.mT.mZ = mZ.mW; out.mT.mW = mT.mW;

	*this = out;
}

void VuMatrix::invert()
{
	float a0 = mX.mX*mY.mY - mX.mY*mY.mX;
	float a1 = mX.mX*mY.mZ - mX.mZ*mY.mX;
	float a2 = mX.mX*mY.mW - mX.mW*mY.mX;
	float a3 = mX.mY*mY.mZ - mX.mZ*mY.mY;
	float a4 = mX.mY*mY.mW - mX.mW*mY.mY;
	float a5 = mX.mZ*mY.mW - mX.mW*mY.mZ;
	float b0 = mZ.mX*mT.mY - mZ.mY*mT.mX;
	float b1 = mZ.mX*mT.mZ - mZ.mZ*mT.mX;
	float b2 = mZ.mX*mT.mW - mZ.mW*mT.mX;
	float b3 = mZ.mY*mT.mZ - mZ.mZ*mT.mY;
	float b4 = mZ.mY*mT.mW - mZ.mW*mT.mY;
	float b5 = mZ.mZ*mT.mW - mZ.mW*mT.mZ;

	float det = a0*b5 - a1*b4 + a2*b3 + a3*b2 - a4*b1 + a5*b0;

	VUASSERT(VuAbs(det) > FLT_EPSILON, "Non-invertable matrix");

	VuMatrix tmp;
	tmp.mX.mX = + mY.mY*b5 - mY.mZ*b4 + mY.mW*b3;
	tmp.mY.mX = - mY.mX*b5 + mY.mZ*b2 - mY.mW*b1;
	tmp.mZ.mX = + mY.mX*b4 - mY.mY*b2 + mY.mW*b0;
	tmp.mT.mX = - mY.mX*b3 + mY.mY*b1 - mY.mZ*b0;
	tmp.mX.mY = - mX.mY*b5 + mX.mZ*b4 - mX.mW*b3;
	tmp.mY.mY = + mX.mX*b5 - mX.mZ*b2 + mX.mW*b1;
	tmp.mZ.mY = - mX.mX*b4 + mX.mY*b2 - mX.mW*b0;
	tmp.mT.mY = + mX.mX*b3 - mX.mY*b1 + mX.mZ*b0;
	tmp.mX.mZ = + mT.mY*a5 - mT.mZ*a4 + mT.mW*a3;
	tmp.mY.mZ = - mT.mX*a5 + mT.mZ*a2 - mT.mW*a1;
	tmp.mZ.mZ = + mT.mX*a4 - mT.mY*a2 + mT.mW*a0;
	tmp.mT.mZ = - mT.mX*a3 + mT.mY*a1 - mT.mZ*a0;
	tmp.mX.mW = - mZ.mY*a5 + mZ.mZ*a4 - mZ.mW*a3;
	tmp.mY.mW = + mZ.mX*a5 - mZ.mZ*a2 + mZ.mW*a1;
	tmp.mZ.mW = - mZ.mX*a4 + mZ.mY*a2 - mZ.mW*a0;
	tmp.mT.mW = + mZ.mX*a3 - mZ.mY*a1 + mZ.mZ*a0;

	float invDet = 1.0f/det;

	mX.mX = invDet*tmp.mX.mX;
	mX.mY = invDet*tmp.mX.mY;
	mX.mZ = invDet*tmp.mX.mZ;
	mX.mW = invDet*tmp.mX.mW;
	mY.mX = invDet*tmp.mY.mX;
	mY.mY = invDet*tmp.mY.mY;
	mY.mZ = invDet*tmp.mY.mZ;
	mY.mW = invDet*tmp.mY.mW;
	mZ.mX = invDet*tmp.mZ.mX;
	mZ.mY = invDet*tmp.mZ.mY;
	mZ.mZ = invDet*tmp.mZ.mZ;
	mZ.mW = invDet*tmp.mZ.mW;
	mT.mX = invDet*tmp.mT.mX;
	mT.mY = invDet*tmp.mT.mY;
	mT.mZ = invDet*tmp.mT.mZ;
	mT.mW = invDet*tmp.mT.mW;
}

void VuMatrix::fastInvert()
{
	VuSwap(mX.mY, mY.mX);
	VuSwap(mX.mZ, mZ.mX);
	VuSwap(mY.mZ, mZ.mY);
	setTrans(transformNormal(-getTrans()));
}

VuMatrix VuMatrix::operator *(const VuMatrix &mat) const
{
	VuMatrix out;

	out.mX = mX.mX*mat.mX + mX.mY*mat.mY + mX.mZ*mat.mZ + mX.mW*mat.mT;
	out.mY = mY.mX*mat.mX + mY.mY*mat.mY + mY.mZ*mat.mZ + mY.mW*mat.mT;
	out.mZ = mZ.mX*mat.mX + mZ.mY*mat.mY + mZ.mZ*mat.mZ + mZ.mW*mat.mT;
	out.mT = mT.mX*mat.mX + mT.mY*mat.mY + mT.mZ*mat.mZ + mT.mW*mat.mT;

	return out;
}

void VuMatrix::operator *=(const VuMatrix &mat)
{
	*this = *this*mat;
}

VuVector2 VuMatrix::transform(const VuVector2 &v) const
{
	VuVector2 out;

	out.mX = mX.mX*v.mX + mY.mX*v.mY + mT.mX;
	out.mY = mX.mY*v.mX + mY.mY*v.mY + mT.mY;

	return out;
}

VuVector3 VuMatrix::transform(const VuVector3 &v) const
{
	VuVector3 out;

	out.mX = mX.mX*v.mX + mY.mX*v.mY + mZ.mX*v.mZ + mT.mX;
	out.mY = mX.mY*v.mX + mY.mY*v.mY + mZ.mY*v.mZ + mT.mY;
	out.mZ = mX.mZ*v.mX + mY.mZ*v.mY + mZ.mZ*v.mZ + mT.mZ;

	return out;
}

VuVector3 VuMatrix::transformCoord(const VuVector3 &v) const
{
	VuVector3 out;

	float x = mX.mX*v.mX + mY.mX*v.mY + mZ.mX*v.mZ + mT.mX;
	float y = mX.mY*v.mX + mY.mY*v.mY + mZ.mY*v.mZ + mT.mY;
	float z = mX.mZ*v.mX + mY.mZ*v.mY + mZ.mZ*v.mZ + mT.mZ;
	float w = mX.mW*v.mX + mY.mW*v.mY + mZ.mW*v.mZ + mT.mW;

	VUASSERT(VuAbs(w) > FLT_EPSILON, "Invalid matrix");

	float invW = 1.0f/w;

	out.mX = x*invW;
	out.mY = y*invW;
	out.mZ = z*invW;

	return out;
}

VuVector3 VuMatrix::transformNormal(const VuVector3 &v) const
{
	VuVector3 out;

	out.mX = mX.mX*v.mX + mY.mX*v.mY + mZ.mX*v.mZ;
	out.mY = mX.mY*v.mX + mY.mY*v.mY + mZ.mY*v.mZ;
	out.mZ = mX.mZ*v.mX + mY.mZ*v.mY + mZ.mZ*v.mZ;

	return out;
}

VuVector4 VuMatrix::transform(const VuVector4 &v) const
{
	VuVector4 out;

	out.mX = mX.mX*v.mX + mY.mX*v.mY + mZ.mX*v.mZ + mT.mX*v.mW;
	out.mY = mX.mY*v.mX + mY.mY*v.mY + mZ.mY*v.mZ + mT.mY*v.mW;
	out.mZ = mX.mZ*v.mX + mY.mZ*v.mY + mZ.mZ*v.mZ + mT.mZ*v.mW;
	out.mW = mX.mW*v.mX + mY.mW*v.mY + mZ.mW*v.mZ + mT.mW*v.mW;

	return out;
}

float VuMatrix::determinant() const
{
	float a0 = mX.mX*mY.mY - mX.mY*mY.mX;
	float a1 = mX.mX*mY.mZ - mX.mZ*mY.mX;
	float a2 = mX.mX*mY.mW - mX.mW*mY.mX;
	float a3 = mX.mY*mY.mZ - mX.mZ*mY.mY;
	float a4 = mX.mY*mY.mW - mX.mW*mY.mY;
	float a5 = mX.mZ*mY.mW - mX.mW*mY.mZ;
	float b0 = mZ.mX*mT.mY - mZ.mY*mT.mX;
	float b1 = mZ.mX*mT.mZ - mZ.mZ*mT.mX;
	float b2 = mZ.mX*mT.mW - mZ.mW*mT.mX;
	float b3 = mZ.mY*mT.mZ - mZ.mZ*mT.mY;
	float b4 = mZ.mY*mT.mW - mZ.mW*mT.mY;
	float b5 = mZ.mZ*mT.mW - mZ.mW*mT.mZ;

	return a0*b5 - a1*b4 + a2*b3 + a3*b2 - a4*b1 + a5*b0;
}

VuMatrix VuMatrix::translation(const VuVector3 &v)
{
	VuMatrix out;

	out.mX.mX = 1;    out.mX.mY = 0;    out.mX.mZ = 0;    out.mX.mW = 0;
	out.mY.mX = 0;    out.mY.mY = 1;    out.mY.mZ = 0;    out.mY.mW = 0;
	out.mZ.mX = 0;    out.mZ.mY = 0;    out.mZ.mZ = 1;    out.mZ.mW = 0;
	out.mT.mX = v.mX; out.mT.mY = v.mY; out.mT.mZ = v.mZ; out.mT.mW = 1;

	return out;
}

VuMatrix VuMatrix::rotationX(float fAngle)
{
	VuMatrix out;

	float sinx, cosx;
	VuSinCos(fAngle, sinx, cosx);

	out.mX.mX = 1; out.mX.mY = 0;     out.mX.mZ = 0;    out.mX.mW = 0;
	out.mY.mX = 0; out.mY.mY = cosx;  out.mY.mZ = sinx; out.mY.mW = 0;
	out.mZ.mX = 0; out.mZ.mY = -sinx; out.mZ.mZ = cosx; out.mZ.mW = 0;
	out.mT.mX = 0; out.mT.mY = 0;     out.mT.mZ = 0;    out.mT.mW = 1;

	return out;
}

VuMatrix VuMatrix::rotationY(float fAngle)
{
	VuMatrix out;

	float siny, cosy;
	VuSinCos(fAngle, siny, cosy);

	out.mX.mX = cosy; out.mX.mY = 0; out.mX.mZ = -siny; out.mX.mW = 0;
	out.mY.mX = 0;    out.mY.mY = 1; out.mY.mZ = 0;     out.mY.mW = 0;
	out.mZ.mX = siny; out.mZ.mY = 0; out.mZ.mZ = cosy;  out.mZ.mW = 0;
	out.mT.mX = 0;    out.mT.mY = 0; out.mT.mZ = 0;     out.mT.mW = 1;

	return out;
}

VuMatrix VuMatrix::rotationZ(float fAngle)
{
	VuMatrix out;

	float sinz, cosz;
	VuSinCos(fAngle, sinz, cosz);

	out.mX.mX = cosz;  out.mX.mY = sinz; out.mX.mZ = 0; out.mX.mW = 0;
	out.mY.mX = -sinz; out.mY.mY = cosz; out.mY.mZ = 0; out.mY.mW = 0;
	out.mZ.mX = 0;     out.mZ.mY = 0;    out.mZ.mZ = 1; out.mZ.mW = 0;
	out.mT.mX = 0;     out.mT.mY = 0;    out.mT.mZ = 0; out.mT.mW = 1;

	return out;
}

VuMatrix VuMatrix::rotationXYZ(const VuVector3 &v)
{
	VuMatrix out;

	float sinx, cosx;
	float siny, cosy;
	float sinz, cosz;
	VuSinCos(v.mX, sinx, cosx);
	VuSinCos(v.mY, siny, cosy);
	VuSinCos(v.mZ, sinz, cosz);

	out.mX.mX = cosy*cosz;
	out.mX.mY = cosy*sinz;
	out.mX.mZ = -siny;
	out.mX.mW = 0;

	out.mY.mX = cosz*sinx*siny - cosx*sinz;
	out.mY.mY = sinx*siny*sinz + cosx*cosz;
	out.mY.mZ = cosy*sinx;
	out.mY.mW = 0;

	out.mZ.mX = cosx*cosz*siny + sinx*sinz;
	out.mZ.mY = cosx*siny*sinz - cosz*sinx;
	out.mZ.mZ = cosx*cosy;
	out.mZ.mW = 0;

	out.mT.mX = 0;
	out.mT.mY = 0;
	out.mT.mZ = 0;
	out.mT.mW = 1.0f;

	return out;
}

VuMatrix VuMatrix::rotationAxis(const VuVector3 &v, float angle)
{
	VuMatrix out;

	const float x = v.mX;
	const float y = v.mY;
	const float z = v.mZ;

	float sin, cos;
	VuSinCos(angle, sin, cos);

	const float t = 1.0f - cos;

	out.mX.mX = cos + t*x*x;
	out.mX.mY = t*x*y + z*sin;
	out.mX.mZ = t*x*z - y*sin;
	out.mX.mW = 0;

	out.mY.mX = t*x*y - z*sin;
	out.mY.mY = cos + t*y*y;
	out.mY.mZ = t*y*z + x*sin;
	out.mY.mW = 0;

	out.mZ.mX = t*x*z + y*sin;
	out.mZ.mY = t*y*z - x*sin;
	out.mZ.mZ = cos + t*z*z;
	out.mZ.mW = 0;

	out.mT.mX = 0;
	out.mT.mY = 0;
	out.mT.mZ = 0;
	out.mT.mW = 1.0f;

	return out;
}

VuMatrix VuMatrix::scaling(const VuVector3 &v)
{
	VuMatrix out;

	out.mX.mX = v.mX; out.mX.mY = 0;    out.mX.mZ = 0;    out.mX.mW = 0;
	out.mY.mX = 0;    out.mY.mY = v.mY; out.mY.mZ = 0;    out.mY.mW = 0;
	out.mZ.mX = 0;    out.mZ.mY = 0;    out.mZ.mZ = v.mZ; out.mZ.mW = 0;
	out.mT.mX = 0;    out.mT.mY = 0;    out.mT.mZ = 0;    out.mT.mW = 1;

	return out;
}

VuMatrix VuMatrix::scaling(float f)
{
	VuMatrix out;

	out.mX.mX = f; out.mX.mY = 0; out.mX.mZ = 0; out.mX.mW = 0;
	out.mY.mX = 0; out.mY.mY = f; out.mY.mZ = 0; out.mY.mW = 0;
	out.mZ.mX = 0; out.mZ.mY = 0; out.mZ.mZ = f; out.mZ.mW = 0;
	out.mT.mX = 0; out.mT.mY = 0; out.mT.mZ = 0; out.mT.mW = 1;

	return out;
}

