//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Font enum property class
// 
//*****************************************************************************

#pragma once

#include "VuEnumProperty.h"
#include "VuEngine/DB/VuFontDB.h"


class VuFontEnumProperty : public VuStringEnumProperty
{
public:
	VuFontEnumProperty(const char *strName, std::string &pValue) : VuStringEnumProperty(strName, pValue) {}

	virtual eType		getType() const { return ENUM_STRING; }

	virtual int			getChoiceCount() const { return VuFontDB::IF()->getFontCount(); }
	virtual const char	*getChoice(int index) const { return VuFontDB::IF()->getFontName(index); }
};
