//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Template Asset class
// 
//*****************************************************************************

#pragma once

#include "VuGenericDataAsset.h"


class VuTemplateAsset : public VuGenericDataAsset
{
	DECLARE_RTTI

protected:
	~VuTemplateAsset() {}

public:
	const std::string		&getEntityType() const { return getDataContainer()["RootEntity"]["type"].asString(); }
	const VuJsonContainer	&getTemplate() const { return getDataContainer()["RootEntity"]["data"]; }

	static void		schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
};
