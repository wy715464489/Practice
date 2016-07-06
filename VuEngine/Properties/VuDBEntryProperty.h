//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  DBEntry property class
// 
//*****************************************************************************

#pragma once

#include "VuProperty.h"
#include "VuEnumProperty.h"

class VuDBAsset;

class VuDBEntryProperty : public VuStringEnumProperty
{
public:

	VuDBEntryProperty(const char *strName, std::string &pValue, const char *strDBName);
	~VuDBEntryProperty();

	virtual int				getChoiceCount() const;
	virtual const char		*getChoice(int index) const;

	const VuJsonContainer	&getEntryData() const;
	const VuJsonContainer	&getDB() const;

	void					reloadDB();

protected:
	VuDBAsset				*mpDBAsset;
};
