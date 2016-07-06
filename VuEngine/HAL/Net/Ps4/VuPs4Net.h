//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 interface class to Network library.
// 
//*****************************************************************************

#pragma once

#include <net.h>
#include "VuEngine/HAL/Net/VuNet.h"


class VuPs4Net : public VuNet
{
public:
	virtual bool init();
	virtual void release();

	// platform-specific functionality
	static VuPs4Net *IF() { return static_cast<VuPs4Net *>(VuNet::IF()); }
	
	bool		lookupAddress(const char *strAddress, VUUINT32 &ipAddr);

private:
	int			mResolverMemPool;
	SceNetId	mResolver;
};
