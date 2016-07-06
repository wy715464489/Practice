//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Network library.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"

class VuEngine;


class VuTcpSocket
{
public:
	VuTcpSocket() : mNonBlocking(false) {}
	virtual ~VuTcpSocket() {}

	static VuTcpSocket	*create(VUUINT16 port);

	// options
	virtual bool		setNonBlocking(bool nonBlocking) = 0;
	virtual bool		setTimeOut(int receiveMS, int sendMS) = 0;

	// server
	virtual bool		listen(int backlog) = 0;
	virtual VuTcpSocket	*accept() = 0;

	// client
	virtual bool		connect(const char *hostName, int port, int timeoutMS) = 0;
	virtual void		disconnect() {}

	// send/receive
	virtual int			send(const void *pData, int dataSize) = 0;
	virtual int			recv(void *pData, int bufSize) = 0;

protected:
	bool				mNonBlocking;
};


class VuNet : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuNet)

protected:
	// called by engine
	friend class VuEngine;
	virtual bool init() { return true; }
};
