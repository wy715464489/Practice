//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Angle property class
// 
//*****************************************************************************

#pragma once

#include "VuBasicProperty.h"


class VuAngleProperty : public VuFloatProperty
{
public:
	VuAngleProperty(const char *strName, float &pValue) : VuFloatProperty(strName, pValue) {}

protected:
	virtual float	transformToNative(const float &formalValue) const	{ return VuDegreesToRadians(formalValue); }	
	virtual float	transformFromNative(const float &nativeValue) const	{ return VuRadiansToDegrees(nativeValue); }
};
