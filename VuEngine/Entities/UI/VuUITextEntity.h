//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  UI Text class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Entities/UI/VuUITextBaseEntity.h"

class VuUITextEntity : public VuUITextBaseEntity
{
	DECLARE_RTTI

public:
	VuUITextEntity();

	const char	*getText();

protected:

	VuRetVal	SetStringID(const VuParams &params);

	std::string	mStringID;
};
