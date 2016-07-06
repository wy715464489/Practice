//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Service
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuList.h"


class VuService : public VuListElement<VuService>
{
public:
	virtual ~VuService() {}
	virtual bool	tick(float fdt) = 0;
};
