//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  D3d9 inline functions for Matrix class
// 
//*****************************************************************************

#include <d3dx9math.h>

#ifdef VUDEBUG
	#pragma comment(lib, "d3dx9d.lib")
#else
	#pragma comment(lib, "d3dx9.lib")
#endif


void VuMatrix::loadIdentity()
{
	D3DXMatrixIdentity((D3DXMATRIX *)this);
}

void VuMatrix::transpose()
{
	D3DXMATRIX out;
	*this = *(VuMatrix *)D3DXMatrixTranspose(&out, (D3DXMATRIX *)this);
}

void VuMatrix::invert()
{
	D3DXMATRIX out;
	D3DXMATRIX *pOut;

	pOut = D3DXMatrixInverse(&out, NULL, (D3DXMATRIX *)this);

	VUASSERT(pOut, "Non-invertable matrix");

	*this = *(VuMatrix *)pOut;
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
	D3DXMATRIX out;
	return *(VuMatrix *)D3DXMatrixMultiply(&out, (D3DXMATRIX *)this, (D3DXMATRIX *)&mat);
}

void VuMatrix::operator *=(const VuMatrix &mat)
{
	D3DXMATRIX out;
	*this = *(VuMatrix *)D3DXMatrixMultiply(&out, (D3DXMATRIX *)this, (D3DXMATRIX *)&mat);
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
	D3DXVECTOR4 out;
	return *(VuVector3 *)D3DXVec3Transform(&out, (D3DXVECTOR3 *)&v, (D3DXMATRIX *)this);
}

VuVector3 VuMatrix::transformCoord(const VuVector3 &v) const
{
	D3DXVECTOR3 out;
	return *(VuVector3 *)D3DXVec3TransformCoord(&out, (D3DXVECTOR3 *)&v, (D3DXMATRIX *)this);
}

VuVector3 VuMatrix::transformNormal(const VuVector3 &v) const
{
	D3DXVECTOR3 out;
	return *(VuVector3 *)D3DXVec3TransformNormal(&out, (D3DXVECTOR3 *)&v, (D3DXMATRIX *)this);
}

VuVector4 VuMatrix::transform(const VuVector4 &v) const
{
	D3DXVECTOR4 out;
	return *(VuVector4 *)D3DXVec4Transform(&out, (D3DXVECTOR4 *)&v, (D3DXMATRIX *)this);
}

float VuMatrix::determinant() const
{
	return D3DXMatrixDeterminant((D3DXMATRIX *)this);
}

VuMatrix VuMatrix::translation(const VuVector3 &v)
{
	D3DXMATRIX out;
	return *(VuMatrix *)D3DXMatrixTranslation(&out, v.mX, v.mY, v.mZ);
}

VuMatrix VuMatrix::rotationX(float fAngle)
{
	D3DXMATRIX out;
	return *(VuMatrix *)D3DXMatrixRotationX(&out, fAngle);
}

VuMatrix VuMatrix::rotationY(float fAngle)
{
	D3DXMATRIX out;
	return *(VuMatrix *)D3DXMatrixRotationY(&out, fAngle);
}

VuMatrix VuMatrix::rotationZ(float fAngle)
{
	D3DXMATRIX out;
	return *(VuMatrix *)D3DXMatrixRotationZ(&out, fAngle);
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
	D3DXMATRIX out;
	return *(VuMatrix *)D3DXMatrixScaling(&out, v.mX, v.mY, v.mZ);
}

VuMatrix VuMatrix::scaling(float f)
{
	D3DXMATRIX out;
	return *(VuMatrix *)D3DXMatrixScaling(&out, f, f, f);
}

