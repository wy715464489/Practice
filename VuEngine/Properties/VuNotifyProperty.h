//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Void property class
// 
//*****************************************************************************

#pragma once

#include "VuProperty.h"


class VuNotifyProperty : public VuProperty
{
public:
	VuNotifyProperty(const char *strName)
		: VuProperty(strName)
	{}

	virtual eType	getType() const { return NOTIFY; }

	virtual void	load(const VuJsonContainer &data) {}
	virtual void	save(VuJsonContainer &data) const {}
	virtual void	load(const VuFastContainer &data) {}

	virtual void	setCurrent(const VuJsonContainer &data, bool notify = true)	{ if ( notify ) notifyWatcher(); }
	virtual void	getCurrent(VuJsonContainer &data) const {}

	virtual void	getDefault(VuJsonContainer &data) const {}
	virtual void	updateDefault() {}

	virtual void	reset() {}
};
