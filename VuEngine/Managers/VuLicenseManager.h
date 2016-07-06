//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  LicenseManager class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"


class VuLicenseManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuLicenseManager)

public:
	virtual bool	init() { return true; }

	virtual bool	isTrial() = 0;
};
