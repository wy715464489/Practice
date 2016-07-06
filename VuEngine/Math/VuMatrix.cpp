//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Matrix class
// 
//*****************************************************************************

#include "VuMatrix.h"


VuMatrix VuMatrix::smIdentityMatrix(VuVector4(1,0,0,0), VuVector4(0,1,0,0), VuVector4(0,0,1,0), VuVector4(0,0,0,1));


//*****************************************************************************
VuVector3 VuMatrix::getEulerAngles() const
{
	float yr = VuASin(VuClamp(-mX.mZ, -1.0f, 1.0f));
	float xr = VuATan2(mY.mZ, mZ.mZ);
	float zr = VuATan2(mX.mY, mX.mX);

	return VuVector3(xr, yr, zr);
}

//*****************************************************************************
void VuMatrix::setEulerAngles(const VuVector3 &v)
{
	loadIdentity();
	rotateXYZ(v);
}
