//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Service Manager
// 
//*****************************************************************************

#include "VuServiceManager.h"
#include "VuService.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Dev/VuDevStat.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuServiceManager, VuServiceManager);


//*****************************************************************************
bool VuServiceManager::init()
{
	VuTickManager::IF()->registerHandler(this, &VuServiceManager::tickServices, "Services");
	VuTickManager::IF()->registerHandler(this, &VuServiceManager::tickPostBuild, "PostBuild");

	return true;
}

//*****************************************************************************
void VuServiceManager::postInit()
{
	if ( VuDevStat::IF() )
		VuDevStat::IF()->addPage("Services", VuRect(50, 10, 40, 40));
}

//*****************************************************************************
void VuServiceManager::release()
{
	// release any active services
	while (mActiveServices.size())
		releaseService(mActiveServices.front());

	while (mActivePfxServices.size())
		releasePfxService(mActivePfxServices.front());

	VuTickManager::IF()->unregisterHandlers(this);
}

//*****************************************************************************
void VuServiceManager::releaseService(VuService *pService)
{
	mActiveServices.remove(pService);
	delete pService;
}

//*****************************************************************************
void VuServiceManager::releasePfxService(VuService *pService)
{
	mActivePfxServices.remove(pService);
	delete pService;
}

//*****************************************************************************
void VuServiceManager::tickServices(float fdt)
{
	VuService *pService = mActiveServices.front();
	while (pService)
	{
		VuService *pNextService = pService->next();

		if (!pService->tick(fdt))
			releaseService(pService);

		pService = pNextService;
	}
}

//*****************************************************************************
void VuServiceManager::tickPostBuild(float fdt)
{
	VuService *pService = mActivePfxServices.front();
	while (pService)
	{
		VuService *pNextService = pService->next();

		if (!pService->tick(fdt))
			releasePfxService(pService);

		pService = pNextService;
	}

	updateDevStats();
}

//*****************************************************************************
void VuServiceManager::updateDevStats()
{
	// dev stats
	if ( VuDevStat::IF() )
	{
		if ( VuDevStatPage *pPage = VuDevStat::IF()->getCurPage() )
		{
			if ( pPage->getName() == "Services" )
			{
				pPage->clear();

				// running services
				{
					pPage->printf("Services: %3d\n", mActiveServices.size());
					pPage->printf("Pfx Services: %3d\n", mActivePfxServices.size());
					pPage->printf("Total Services: %3d\n", mActiveServices.size() + mActivePfxServices.size());
				}
			}
		}
	}
}
