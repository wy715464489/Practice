//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ServiceManager class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Containers/VuList.h"

class VuService;


class VuServiceManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuServiceManager)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init();
	virtual void postInit();
	virtual void release();

public:
	template <class T>
	T			*createService();
	void		releaseService(VuService *pService);

	template <class T>
	T			*createPfxService();
	void		releasePfxService(VuService *pService);

private:
	void		tickServices(float fdt);
	void		tickPostBuild(float fdt);
	void		updateDevStats();

	typedef VuList<VuService> Services;

	Services	mActiveServices;
	Services	mActivePfxServices;
};


#include "VuServiceManager.inl"