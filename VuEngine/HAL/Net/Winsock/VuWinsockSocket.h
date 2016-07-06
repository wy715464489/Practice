//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Winsock socket.
//
//*****************************************************************************

#pragma once

#include "VuEngine/HAL/Net/VuNet.h"

class VuWinsockTcpSocket : public VuTcpSocket
{
public:
	virtual ~VuWinsockTcpSocket();

	virtual bool		setNonBlocking(bool nonBlocking);
	virtual bool		setTimeOut(int receiveMS, int sendMS);
	virtual bool		listen(int backlog);
	virtual VuTcpSocket	*accept();
	virtual bool		connect(const char *hostName, int port, int timeoutMS);
	virtual void		disconnect();
	virtual int			send(const void *pData, int dataSize);
	virtual int			recv(void *pData, int bufSize);

	static bool			lookupAddress(const char *strAddress, VUUINT32 &ipAddr);

	SOCKET				mSocket;
};
