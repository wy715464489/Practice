//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Percentage property class
// 
//*****************************************************************************

#pragma once

#include "VuBasicProperty.h"


class VuPercentageProperty : public VuFloatProperty
{
public:
	VuPercentageProperty(const char *strName, float &pValue) : VuFloatProperty(strName, pValue) {}

protected:
	virtual float	transformToNative(const float &formalValue) const	{ return formalValue/100.0f; }	
	virtual float	transformFromNative(const float &nativeValue) const	{ return nativeValue*100.0f; }
};
