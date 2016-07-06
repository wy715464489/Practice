//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Rotation3d property class
// 
//*****************************************************************************

#pragma once

#include "VuBasicProperty.h"


class VuRotation3dProperty : public VuVector3Property
{
public:
	VuRotation3dProperty(const char *strName, VuVector3 &pValue) : VuVector3Property(strName, pValue) {}

protected:
	virtual VuVector3	transformToNative(const VuVector3 &formalValue) const	{ return VuVector3(VuDegreesToRadians(formalValue.mX), VuDegreesToRadians(formalValue.mY), VuDegreesToRadians(formalValue.mZ));}	
	virtual VuVector3	transformFromNative(const VuVector3 &nativeValue) const	{ return VuVector3(VuRadiansToDegrees(nativeValue.mX), VuRadiansToDegrees(nativeValue.mY), VuRadiansToDegrees(nativeValue.mZ)); }
};
