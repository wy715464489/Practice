//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Xb1 interface class for Net.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Net/Winsock/VuWinsockSocket.h"


class VuXb1Net : public VuNet
{
protected:
	virtual bool init();
	virtual void release();
};


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuNet, VuXb1Net);


//*****************************************************************************
bool VuXb1Net::init()
{
	WSADATA wsaData;
	if ( WSAStartup(MAKEWORD(2,2), &wsaData) )
		return VUERROR("Could not start socket networking");

	return true;
}

//*****************************************************************************
void VuXb1Net::release()
{
	WSACleanup();
}
