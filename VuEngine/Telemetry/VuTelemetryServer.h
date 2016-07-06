//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Telemetry Server class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"

class VuEngine;

class VuTelemetryServer : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuTelemetryServer)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init() = 0;
};
