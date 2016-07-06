//*****************************************************************************
//
//  Copyright (c) 2014-2014 Ralf Knoesel
//  Confidential Trade Secrets
// 
//  String property class
// 
//*****************************************************************************

#pragma once

#include "VuProperty.h"


class VuStringProperty : public VuProperty
{
public:
	VuStringProperty(const char *strName, std::string &value) : VuProperty(strName), mDefaultValue(value), mInitialValue(value), mValue(value) {}

	virtual eType	getType() const { return STRING; }

	virtual void	load(const VuJsonContainer &data);
	virtual void	save(VuJsonContainer &data) const;
	virtual void	load(const VuFastContainer &data);

	virtual void	setCurrent(const VuJsonContainer &data, bool notify = true);
	virtual void	getCurrent(VuJsonContainer &data) const;

	virtual void	getDefault(VuJsonContainer &data) const;
	virtual void	updateDefault()	{ mDefaultValue = mValue; }

	virtual void	reset() { mValue = mInitialValue; }

protected:
	// notifies derived classes that the value has changed
	virtual void	onValueChanged() {}			

	std::string		mDefaultValue;
	std::string		mInitialValue;
	std::string		&mValue;
};

