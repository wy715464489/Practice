//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Bit Field property class
// 
//*****************************************************************************

#pragma once

#include "VuProperty.h"


class VuBitFieldProperty : public VuProperty
{
public:
	VuBitFieldProperty(const char *strName, VUUINT32 &value, VUUINT32 mask);

	virtual eType	getType() const { return BOOLEAN; }

	virtual void		load(const VuJsonContainer &data);
	virtual void		save(VuJsonContainer &data) const;
	virtual void		load(const VuFastContainer &data);

	virtual void		setCurrent(const VuJsonContainer &data, bool notify = true);
	virtual void		getCurrent(VuJsonContainer &data) const;

	virtual void		getDefault(VuJsonContainer &data) const;
	virtual void		updateDefault()	{ mDefaultValue = readBitField(); }

	virtual void		reset() { writeBitField(mInitialValue); }

	bool				mDefaultValue;
	bool				mInitialValue;
	VUUINT32			&mValue;
	VUUINT32			mMask;

private:
	bool				readBitField() const;
	void				writeBitField(bool value);
};
