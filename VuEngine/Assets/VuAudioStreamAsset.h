//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Audio Stream Asset class
// 
//*****************************************************************************

#pragma once

#include "VuGenericAsset.h"


class VuAudioStreamAsset : public VuGenericAsset
{
	DECLARE_RTTI

public:
	static void		schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema);
};
