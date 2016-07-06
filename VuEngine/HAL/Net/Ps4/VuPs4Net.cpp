//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 interface class to Network library.
// 
//*****************************************************************************

#include <net.h>
#include "VuPs4Net.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuNet, VuPs4Net);


//*****************************************************************************
bool VuPs4Net::init()
{
	mResolverMemPool = sceNetPoolCreate("VuNet", 4096, 0);
	if ( mResolverMemPool < 0 )
		return false;

	mResolver = sceNetResolverCreate("VuNet", mResolverMemPool, 0);
	if ( mResolver < 0 )
		return false;

	return true;
}

//*****************************************************************************
void VuPs4Net::release()
{
	sceNetResolverAbort(mResolver, 0);
	sceNetResolverDestroy(mResolver);
	sceNetPoolDestroy(mResolverMemPool);
}

//*****************************************************************************
bool VuPs4Net::lookupAddress(const char *strAddress, VUUINT32 &ipAddr)
{
	SceNetInAddr sceAddr;
	if ( sceNetResolverStartNtoa(mResolver, strAddress, &sceAddr, 0, 0, 0) >= 0 )
	{
		ipAddr = sceNetNtohl(sceAddr.s_addr);
		return true;
	}

	int addr[4] = {0,0,0,0};
	if ( sscanf(strAddress, "%d.%d.%d.%d", &addr[0], &addr[1], &addr[2], &addr[3]) == 4 )
	{
		ipAddr = (addr[0]<<24) | (addr[1]<<16) | (addr[2]<<8) | (addr[3]<<0);
		return true;
	}

	return false;
}
