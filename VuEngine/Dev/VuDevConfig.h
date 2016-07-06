//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to the config file.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Json/VuJsonContainer.h"


#if VU_DISABLE_DEV_CONFIG

	class VuDevConfig : public VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDevConfig)
	protected:
		friend class VuEngine;
		virtual bool init() { return true; }
	public:
		const VuJsonContainer	&getParam(const char *strName) const { return VuJsonContainer::null; }
		bool					hasParam(const char *strName) const { return false; }
	};

#else

	class VuDevConfig : public VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDevConfig)

	protected:
		// called by engine
		friend class VuEngine;
		virtual bool init();

	public:
		// config access
		const VuJsonContainer	&getParam(const char *strName) const { return mConfig[strName]; }
		bool					hasParam(const char *strName) const { return mConfig.hasMember(strName); }

	private:
		VuJsonContainer	mConfig;
	};

#endif // VU_DISABLE_DEV_CONFIG
