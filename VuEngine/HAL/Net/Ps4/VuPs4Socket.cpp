//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  PS4 socket
// 
//*****************************************************************************

#include <net.h>
#include "VuPs4Net.h"


class VuPs4TcpSocket : public VuTcpSocket
{
public:
	virtual ~VuPs4TcpSocket();

	virtual bool		setNonBlocking(bool nonBlocking);
	virtual bool		setTimeOut(int receiveMS, int sendMS);
	virtual bool		listen(int backlog);
	virtual VuTcpSocket	*accept();
	virtual bool		connect(const char *hostName, int port, int timeoutMS);
	virtual int			send(const void *pData, int dataSize);
	virtual int			recv(void *pData, int bufSize);

	int					mSocket;
};


//*****************************************************************************
VuTcpSocket *VuTcpSocket::create(VUUINT16 port)
{
	// create socket
	int socket = sceNetSocket("VuNet", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
	if ( socket < 0 )
	{
		VUWARNING("Could not create socket!");
		return VUNULL;
	}

	// bind socket to port
	SceNetSockaddrIn addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_len = sizeof(addr);
	addr.sin_family = SCE_NET_AF_INET;
	addr.sin_port = sceNetHtons(port);
	addr.sin_addr.s_addr = SCE_NET_INADDR_ANY;
	if ( sceNetBind(socket, (SceNetSockaddr *)&addr, sizeof(addr)) != SCE_OK )
	{
		VUWARNING("Could not bind socket!");
		return VUNULL;
	}

	VuPs4TcpSocket *pSocket = new VuPs4TcpSocket();
	pSocket->mSocket = socket;

	return pSocket;
}

//*****************************************************************************
VuPs4TcpSocket::~VuPs4TcpSocket()
{
	sceNetShutdown(mSocket, SCE_NET_SHUT_RDWR);
	sceNetSocketClose(mSocket);
}

//*****************************************************************************
bool VuPs4TcpSocket::setNonBlocking(bool nonBlocking)
{
	int val = nonBlocking;
	if ( sceNetSetsockopt(mSocket, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &val, sizeof(val)) != SCE_OK )
		return false;
	
	return true;
}

//*****************************************************************************
bool VuPs4TcpSocket::setTimeOut(int receiveMS, int sendMS)
{
	int val;

	val = receiveMS*1000;
	if ( sceNetSetsockopt(mSocket, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVTIMEO, &val, sizeof(val)) != SCE_OK )
		return false;

	val = sendMS*1000;
	if ( sceNetSetsockopt(mSocket, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVTIMEO, &val, sizeof(val)) != SCE_OK )
		return false;

	return true;
}

//*****************************************************************************
bool VuPs4TcpSocket::listen(int backlog)
{
	return sceNetListen(mSocket, backlog) == SCE_OK;
}

//*****************************************************************************
VuTcpSocket *VuPs4TcpSocket::accept()
{
	int socket = sceNetAccept(mSocket, VUNULL, VUNULL);
	if ( socket < 0 )
		return VUNULL;

	VuPs4TcpSocket *pSocket = new VuPs4TcpSocket;
	pSocket->mSocket = socket;

	return pSocket;
}

//*****************************************************************************
bool VuPs4TcpSocket::connect(const char *hostName, int port, int timeoutMS)
{
	// put socket into nonblocking mode
	bool wasNonBlocking = mNonBlocking;
	if ( !setNonBlocking(true) )
		return false;

	VUUINT32 ipAddr;
	if ( !VuPs4Net::IF()->lookupAddress(hostName, ipAddr) )
		return false;

	SceNetSockaddrIn addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_len = sizeof(addr);
	addr.sin_family = SCE_NET_AF_INET;
	addr.sin_port = sceNetHtons(port);
	addr.sin_addr.s_addr = sceNetHtonl(ipAddr);

	int error = sceNetConnect(mSocket, (SceNetSockaddr*)&addr, sizeof(addr));
	if (error < 0)
	{
		if (error != SCE_NET_ERROR_EINPROGRESS)
		{
			VUPRINTF("Error: Could not connect to socket %d, %s : %d\n", mSocket, hostName, port);
			return false;
		}
		else
		{
			// Allow in-progress connections to complete on their own
		}
	}

	// restore nonblocking mode
	setNonBlocking(wasNonBlocking);

	return true;
}

//*****************************************************************************
int VuPs4TcpSocket::send(const void *pData, int dataSize)
{
	int bytesSent = sceNetSend(mSocket, (char *)pData, dataSize, 0);

	if (bytesSent < 0)
	{
//		VUPRINTF("Error: sceNetSend() returned %0x.\n", bytesSent);

		return 0;
	}

	return bytesSent;
}

//*****************************************************************************
int VuPs4TcpSocket::recv(void *pData, int bufSize)
{
	int bytesReceived = sceNetRecv(mSocket, (char *)pData, bufSize, 0);

	if (bytesReceived < 0)
	{
//		VUPRINTF("Error: sceNetRecv() returned %0x.\n", bytesReceived);

		return 0;
	}

	return bytesReceived;
}
