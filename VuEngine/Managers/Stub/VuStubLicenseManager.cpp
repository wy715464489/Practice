//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Stub License manager
//
//*****************************************************************************

#include "VuEngine/Managers/VuLicenseManager.h"
#include "VuEngine/Dev/VuDevConfig.h"
#include "VuEngine/Dev/VuDevMenu.h"


class VuStubLicenseManager : public VuLicenseManager
{
public:
	VuStubLicenseManager();

	virtual bool	isTrial() { return mTrial; }

	bool		mTrial;
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuLicenseManager, VuStubLicenseManager);


//*****************************************************************************
VuStubLicenseManager::VuStubLicenseManager()
{
	mTrial = VuDevConfig::IF()->getParam("Trial").asBool();
	VuDevMenu::IF()->addBool("Dev/Trial", mTrial);
}
