//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI event enum property class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Properties/VuEnumProperty.h"


//*****************************************************************************
class VuUIEventEnumProperty : public VuStringEnumProperty
{
public:
	VuUIEventEnumProperty(const char *strName, std::string &pValue) : VuStringEnumProperty(strName, pValue) {}

	virtual int			getChoiceCount() const;
	virtual const char	*getChoice(int index) const;
};
