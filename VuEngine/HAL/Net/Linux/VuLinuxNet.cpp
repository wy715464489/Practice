//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Network library.
// 
//*****************************************************************************

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "VuEngine/HAL/Net/VuNet.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuNet, VuNet);


class VuLinuxTcpSocket : public VuTcpSocket
{
public:
	virtual ~VuLinuxTcpSocket();

	virtual bool		setNonBlocking(bool nonBlocking);
	virtual bool		setTimeOut(int receiveMS, int sendMS);
	virtual bool		listen(int backlog);
	virtual VuTcpSocket	*accept();
	virtual bool		connect(const char *hostName, int port, int timeoutMS);
	virtual int			send(const void *pData, int dataSize);
	virtual int			recv(void *pData, int bufSize);

	static bool			lookupAddress(const char *strAddress, VUUINT32 &ipAddr);

	int					mSocket;
};


//*****************************************************************************
VuTcpSocket *VuTcpSocket::create(VUUINT16 port)
{
	// create socket
	int socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( socket == -1 )
	{
		VUWARNING("Could not create socket!");
		return VUNULL;
	}

	// bind socket to port
	sockaddr_in myAddr;
	memset(&myAddr, 0, sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(port);
	myAddr.sin_addr.s_addr = INADDR_ANY;
	if ( bind(socket, (sockaddr *)&myAddr, sizeof(myAddr)) == -1 )
	{
		VUWARNING("Could not bind socket!");
		return VUNULL;
	}

	VuLinuxTcpSocket *pSocket = new VuLinuxTcpSocket();
	pSocket->mSocket = socket;

	return pSocket;
}

//*****************************************************************************
VuLinuxTcpSocket::~VuLinuxTcpSocket()
{
	shutdown(mSocket, SHUT_RDWR);
	close(mSocket);
}

//*****************************************************************************
bool VuLinuxTcpSocket::setNonBlocking(bool nonBlocking)
{
	unsigned long val = nonBlocking;
	if ( ioctl(mSocket, FIONBIO, &val) == -1 )
		return false;

	return true;
}

//*****************************************************************************
bool VuLinuxTcpSocket::setTimeOut(int receiveMS, int sendMS)
{
	struct timeval tv;

	tv.tv_sec = receiveMS/1000;
	tv.tv_usec = (receiveMS%1000)*1000;
	if ( setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, (const void *)&tv, sizeof(tv)) == -1 )
		return false;

	tv.tv_sec = sendMS/1000;
	tv.tv_usec = (sendMS%1000)*1000;
	if ( setsockopt(mSocket, SOL_SOCKET, SO_SNDTIMEO, (const void *)&tv, sizeof(tv)) == -1 )
		return false;

	return true;
}

//*****************************************************************************
bool VuLinuxTcpSocket::listen(int backlog)
{
	return ::listen(mSocket, backlog) == 0;
}

//*****************************************************************************
VuTcpSocket *VuLinuxTcpSocket::accept()
{
	int socket = ::accept(mSocket, VUNULL, VUNULL);
	if ( socket == -1 )
		return VUNULL;

	VuLinuxTcpSocket *pSocket = new VuLinuxTcpSocket;
	pSocket->mSocket = socket;

	return pSocket;
}

//*****************************************************************************
bool VuLinuxTcpSocket::connect(const char *hostName, int port, int timeoutMS)
{
	// put socket into nonblocking mode
	bool wasNonBlocking = mNonBlocking;
	if ( !setNonBlocking(true) )
		return false;

	VUUINT32 ipAddr;
	if ( !lookupAddress(hostName, ipAddr) )
		return false;

	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(ipAddr);

	::connect(mSocket, (sockaddr *)&addr, sizeof(addr));

	fd_set writeFDS;
	FD_ZERO(&writeFDS);
	FD_SET(mSocket, &writeFDS);

	struct timeval timeVal;
	timeVal.tv_sec = timeoutMS/1000;
	timeVal.tv_usec = (timeoutMS%1000)*1000;

	int selRet = ::select(mSocket + 1, VUNULL, &writeFDS, VUNULL, &timeVal);

	// restore nonblocking mode
	setNonBlocking(wasNonBlocking);

	return (selRet != -1) && (selRet != 0);
}

//*****************************************************************************
int VuLinuxTcpSocket::send(const void *pData, int dataSize)
{
	int bytesSent = ::send(mSocket, (char *)pData, dataSize, 0);

	return bytesSent;
}

//*****************************************************************************
int VuLinuxTcpSocket::recv(void *pData, int bufSize)
{
	int bytesReceived = ::recv(mSocket, (char *)pData, bufSize, 0);

	return bytesReceived;
}

//*****************************************************************************
bool VuLinuxTcpSocket::lookupAddress(const char *strAddress, VUUINT32 &ipAddr)
{
	addrinfo hints, *pResult;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	if ( getaddrinfo(strAddress, NULL, &hints, &pResult) == 0 )
	{
		ipAddr = ntohl(((sockaddr_in *)pResult->ai_addr)->sin_addr.s_addr);
		freeaddrinfo(pResult);
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
