//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Network library.
// 
//*****************************************************************************

#include "VuWin32Net.h"
#include "VuEngine/HAL/Net/Winsock/VuWinsockSocket.h"


// libs
#pragma comment(lib, "ws2_32.lib")


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuNet, VuWin32Net);


//*****************************************************************************
bool VuWin32Net::init()
{
	WSADATA wsaData;
	if ( WSAStartup(MAKEWORD(2,2), &wsaData) )
		return VUERROR("Could not start socket networking");

	return true;
}

//*****************************************************************************
void VuWin32Net::release()
{
	WSACleanup();
}
