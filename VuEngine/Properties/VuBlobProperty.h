//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Blob property class
// 
//*****************************************************************************

#pragma once

#include "VuProperty.h"
#include "VuEngine/Containers/VuArray.h"


class VuBlobProperty : public VuProperty
{
public:
	VuBlobProperty(const char *strName, VuArray<VUBYTE> &value);

	virtual eType	getType() const { return BLOB; }

	virtual void		load(const VuJsonContainer &data);
	virtual void		save(VuJsonContainer &data) const;
	virtual void		load(const VuFastContainer &data);

	virtual void		setCurrent(const VuJsonContainer &data, bool notify = true);
	virtual void		getCurrent(VuJsonContainer &data) const;

	virtual void		getDefault(VuJsonContainer &data) const {}
	virtual void		updateDefault()	{}

	virtual void		reset() {}

	VuArray<VUBYTE>	&mValue;
};
