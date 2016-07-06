//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Accelerometer hardware abstration layer
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"

class VuEngine;
class VuVector3;


class VuAccel : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuAccel)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init() { return true; }

public:
	// return acceleration in G
	virtual bool	getAccel(VuVector3 &accel) = 0;
};