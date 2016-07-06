//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Group Entity.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Entities/VuEntity.h"

class Vu3dLayoutComponent;


class VuGroupEntity : public VuEntity
{
	DECLARE_RTTI

public:
	VuGroupEntity();

	bool	isCollapsed() const { return mCollapsed; }

protected:
	// components
	Vu3dLayoutComponent	*mp3dLayoutComponent;

	// properties
	bool	mCollapsed;
};
