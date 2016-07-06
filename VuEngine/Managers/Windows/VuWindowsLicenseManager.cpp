//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Windows License manager
//
//*****************************************************************************

#include "VuEngine/Managers/VuLicenseManager.h"

using namespace Windows::ApplicationModel::Store;


class VuWindowsLicenseManager : public VuLicenseManager
{
protected:
	virtual bool	isTrial();
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuLicenseManager, VuWindowsLicenseManager);


//*****************************************************************************
bool VuWindowsLicenseManager::isTrial()
{
#ifdef VURETAIL
	LicenseInformation^ licenseInformation = CurrentApp::LicenseInformation;
#else
	LicenseInformation^ licenseInformation = CurrentAppSimulator::LicenseInformation;
#endif

	if ( !licenseInformation->IsActive )
		return true;

	return licenseInformation->IsTrial;
}