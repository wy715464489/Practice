//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  String Asset class
// 
//*****************************************************************************

#pragma once

#include "VuGenericDataAsset.h"


class VuStringAsset : public VuGenericDataAsset
{
	DECLARE_RTTI

protected:
	~VuStringAsset() {}

public:
	static void		schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
};
