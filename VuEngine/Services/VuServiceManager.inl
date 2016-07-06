//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  ServiceManager inline functionality
// 
//*****************************************************************************


//*****************************************************************************
template <class T>
T *VuServiceManager::createService()
{
	T *pService = new T;
	mActiveServices.push_back(pService);
	return pService;
}

//*****************************************************************************
template <class T>
T *VuServiceManager::createPfxService()
{
	T *pService = new T;
	mActivePfxServices.push_back(pService);
	return pService;
}
