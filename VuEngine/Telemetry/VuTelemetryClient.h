//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Telemetry Client class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"

class VuEngine;
class VuJsonContainer;

class VuTelemetryClient : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuTelemetryClient)

protected:

	// called by engine
	friend class VuEngine;
	virtual bool init() = 0;

public:
	virtual bool	getNamespace(const std::string &strAddress, VuJsonContainer &namespaceData) = 0;
	virtual bool	getProperties(const std::string &strAddress, const std::string &strLongName, VuJsonContainer &propertyData) = 0;
	virtual bool	setProperties(const std::string &strAddress, const std::string &strLongName, const VuJsonContainer &propertyData) = 0;
};
