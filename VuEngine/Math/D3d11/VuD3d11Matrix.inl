//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Generic inline functions for Matrix class
// 
//*****************************************************************************

#include <DirectXMath.h>
#include <float.h>


void VuMatrix::loadIdentity()
{
	DirectX::XMMATRIX temp = DirectX::XMMatrixIdentity();

	_mm_storeu_ps(&mX.mX, temp.r[0]);
	_mm_storeu_ps(&mY.mX, temp.r[1]);
	_mm_storeu_ps(&mZ.mX, temp.r[2]);
	_mm_storeu_ps(&mT.mX, temp.r[3]);
}

void VuMatrix::transpose()
{
	DirectX::XMMATRIX temp;
	temp.r[0] = _mm_loadu_ps(&mX.mX);
	temp.r[1] = _mm_loadu_ps(&mY.mX);
	temp.r[2] = _mm_loadu_ps(&mZ.mX);
	temp.r[3] = _mm_loadu_ps(&mT.mX);

	temp = XMMatrixTranspose(temp);

	_mm_storeu_ps(&mX.mX, temp.r[0]);
	_mm_storeu_ps(&mY.mX, temp.r[1]);
	_mm_storeu_ps(&mZ.mX, temp.r[2]);
	_mm_storeu_ps(&mT.mX, temp.r[3]);
}

void VuMatrix::invert()
{
	DirectX::XMMATRIX temp;
	temp.r[0] = _mm_loadu_ps(&mX.mX);
	temp.r[1] = _mm_loadu_ps(&mY.mX);
	temp.r[2] = _mm_loadu_ps(&mZ.mX);
	temp.r[3] = _mm_loadu_ps(&mT.mX);

	temp = XMMatrixInverse(VUNULL, temp);

	_mm_storeu_ps(&mX.mX, temp.r[0]);
	_mm_storeu_ps(&mY.mX, temp.r[1]);
	_mm_storeu_ps(&mZ.mX, temp.r[2]);
	_mm_storeu_ps(&mT.mX, temp.r[3]);
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
	DirectX::XMMATRIX m1;
	m1.r[0] = _mm_loadu_ps(&mX.mX);
	m1.r[1] = _mm_loadu_ps(&mY.mX);
	m1.r[2] = _mm_loadu_ps(&mZ.mX);
	m1.r[3] = _mm_loadu_ps(&mT.mX);

	DirectX::XMMATRIX m2;
	m2.r[0] = _mm_loadu_ps(&mat.mX.mX);
	m2.r[1] = _mm_loadu_ps(&mat.mY.mX);
	m2.r[2] = _mm_loadu_ps(&mat.mZ.mX);
	m2.r[3] = _mm_loadu_ps(&mat.mT.mX);

	return (VuMatrix &)XMMatrixMultiply(m1, m2);
}

void VuMatrix::operator *=(const VuMatrix &mat)
{
	DirectX::XMMATRIX m1;
	m1.r[0] = _mm_loadu_ps(&mX.mX);
	m1.r[1] = _mm_loadu_ps(&mY.mX);
	m1.r[2] = _mm_loadu_ps(&mZ.mX);
	m1.r[3] = _mm_loadu_ps(&mT.mX);

	DirectX::XMMATRIX m2;
	m2.r[0] = _mm_loadu_ps(&mat.mX.mX);
	m2.r[1] = _mm_loadu_ps(&mat.mY.mX);
	m2.r[2] = _mm_loadu_ps(&mat.mZ.mX);
	m2.r[3] = _mm_loadu_ps(&mat.mT.mX);

	DirectX::XMMATRIX out = XMMatrixMultiply(m1, m2);

	_mm_storeu_ps(&mX.mX, out.r[0]);
	_mm_storeu_ps(&mY.mX, out.r[1]);
	_mm_storeu_ps(&mZ.mX, out.r[2]);
	_mm_storeu_ps(&mT.mX, out.r[3]);
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
	DirectX::XMMATRIX tempM;
	tempM.r[0] = _mm_loadu_ps(&mX.mX);
	tempM.r[1] = _mm_loadu_ps(&mY.mX);
	tempM.r[2] = _mm_loadu_ps(&mZ.mX);
	tempM.r[3] = _mm_loadu_ps(&mT.mX);

	DirectX::XMVECTOR tempV;
	tempV = _mm_loadu_ps(&v.mX);

	return (VuVector3 &)XMVector3Transform(tempV, tempM);
}

VuVector3 VuMatrix::transformCoord(const VuVector3 &v) const
{
	DirectX::XMMATRIX tempM;
	tempM.r[0] = _mm_loadu_ps(&mX.mX);
	tempM.r[1] = _mm_loadu_ps(&mY.mX);
	tempM.r[2] = _mm_loadu_ps(&mZ.mX);
	tempM.r[3] = _mm_loadu_ps(&mT.mX);

	DirectX::XMVECTOR tempV;
	tempV = _mm_loadu_ps(&v.mX);

	return (VuVector3 &)XMVector3TransformCoord(tempV, tempM);
}

VuVector3 VuMatrix::transformNormal(const VuVector3 &v) const
{
	DirectX::XMMATRIX tempM;
	tempM.r[0] = _mm_loadu_ps(&mX.mX);
	tempM.r[1] = _mm_loadu_ps(&mY.mX);
	tempM.r[2] = _mm_loadu_ps(&mZ.mX);
	tempM.r[3] = _mm_loadu_ps(&mT.mX);

	DirectX::XMVECTOR tempV;
	tempV = _mm_loadu_ps(&v.mX);

	return (VuVector3 &)XMVector3TransformNormal(tempV, tempM);
}

VuVector4 VuMatrix::transform(const VuVector4 &v) const
{
	DirectX::XMMATRIX tempM;
	tempM.r[0] = _mm_loadu_ps(&mX.mX);
	tempM.r[1] = _mm_loadu_ps(&mY.mX);
	tempM.r[2] = _mm_loadu_ps(&mZ.mX);
	tempM.r[3] = _mm_loadu_ps(&mT.mX);

	DirectX::XMVECTOR tempV;
	tempV = _mm_loadu_ps(&v.mX);

	return (VuVector4 &)XMVector4Transform(tempV, tempM);
}

float VuMatrix::determinant() const
{
	DirectX::XMMATRIX temp;
	temp.r[0] = _mm_loadu_ps(&mX.mX);
	temp.r[1] = _mm_loadu_ps(&mY.mX);
	temp.r[2] = _mm_loadu_ps(&mZ.mX);
	temp.r[3] = _mm_loadu_ps(&mT.mX);

	DirectX::XMVECTOR out = XMMatrixDeterminant(temp);
	float det;
	_mm_store_ss(&det, out);
	return det;
}

VuMatrix VuMatrix::translation(const VuVector3 &v)
{
	return (VuMatrix &)DirectX::XMMatrixTranslation(v.mX, v.mY, v.mZ);
}

VuMatrix VuMatrix::rotationX(float fAngle)
{
	return (VuMatrix &)DirectX::XMMatrixRotationX(fAngle);
}

VuMatrix VuMatrix::rotationY(float fAngle)
{
	return (VuMatrix &)DirectX::XMMatrixRotationY(fAngle);
}

VuMatrix VuMatrix::rotationZ(float fAngle)
{
	return (VuMatrix &)DirectX::XMMatrixRotationZ(fAngle);
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
	return (VuMatrix &)DirectX::XMMatrixScaling(v.mX, v.mY, v.mZ);
}

VuMatrix VuMatrix::scaling(float f)
{
	return (VuMatrix &)DirectX::XMMatrixScaling(f, f, f);
}
