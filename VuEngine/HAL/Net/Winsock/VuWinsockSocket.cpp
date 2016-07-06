//*****************************************************************************
//
//  Copyright (c) 2008-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Winsock socket.
//
//*****************************************************************************

#include "VuWinsockSocket.h"


//*****************************************************************************
VuTcpSocket *VuTcpSocket::create(VUUINT16 port)
{
	// create socket
	SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( socket == INVALID_SOCKET )
	{
		VUWARNING("Could not create socket!");
		return VUNULL;
	}

	// bind socket to port
	sockaddr_in myAddr;
	memset(&myAddr, 0, sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(port);
	myAddr.sin_addr.s_addr = ADDR_ANY;
	if ( bind(socket, (sockaddr *)&myAddr, sizeof(myAddr)) == SOCKET_ERROR )
	{
		VUWARNING("Could not bind socket!");
		return VUNULL;
	}

	VuWinsockTcpSocket *pSocket = new VuWinsockTcpSocket();
	pSocket->mSocket = socket;

	return pSocket;
}

//*****************************************************************************
VuWinsockTcpSocket::~VuWinsockTcpSocket()
{
	shutdown(mSocket, SD_BOTH);
	closesocket(mSocket);
}

//*****************************************************************************
bool VuWinsockTcpSocket::setNonBlocking(bool nonBlocking)
{
	if ( mNonBlocking != nonBlocking )
	{
		unsigned long val = nonBlocking;
		if ( ioctlsocket(mSocket, FIONBIO, &val) == SOCKET_ERROR )
			return false;

		mNonBlocking = nonBlocking;
	}

	return true;
}

//*****************************************************************************
bool VuWinsockTcpSocket::setTimeOut(int receiveMS, int sendMS)
{
	DWORD timeout = receiveMS;
	if ( setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) == SOCKET_ERROR )
		return false;

	timeout = sendMS;
	if ( setsockopt(mSocket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout)) == SOCKET_ERROR )
		return false;

	return true;
}

//*****************************************************************************
bool VuWinsockTcpSocket::listen(int backlog)
{
	return ::listen(mSocket, backlog) == 0;
}

//*****************************************************************************
VuTcpSocket *VuWinsockTcpSocket::accept()
{
	SOCKET socket = ::accept(mSocket, VUNULL, VUNULL);
	if ( socket == INVALID_SOCKET )
		return VUNULL;

	VuWinsockTcpSocket *pSocket = new VuWinsockTcpSocket;
	pSocket->mSocket = socket;

	return pSocket;
}

//*****************************************************************************
bool VuWinsockTcpSocket::connect(const char *hostName, int port, int timeoutMS)
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
	addr.sin_port = (USHORT)htons((u_short)port);
	addr.sin_addr.s_addr = htonl(ipAddr);

	::connect(mSocket, (sockaddr *)&addr, sizeof(addr));

	fd_set writeFDS;
	FD_ZERO(&writeFDS);
	FD_SET(mSocket, &writeFDS);

	struct timeval timeVal;
	timeVal.tv_sec = timeoutMS/1000;
	timeVal.tv_usec = (timeoutMS%1000)*1000;

	int selRet = ::select(0, VUNULL, &writeFDS, VUNULL, &timeVal);

	// restore nonblocking mode
	setNonBlocking(wasNonBlocking);

	return (selRet != SOCKET_ERROR) && (selRet != 0);
}

//*****************************************************************************
void VuWinsockTcpSocket::disconnect()
{
	closesocket(mSocket);
	mSocket = 0;
}

//*****************************************************************************
int VuWinsockTcpSocket::send(const void *pData, int dataSize)
{
	int bytesSent = ::send(mSocket, (char *)pData, dataSize, 0);

	return bytesSent;
}

//*****************************************************************************
int VuWinsockTcpSocket::recv(void *pData, int bufSize)
{
	int bytesReceived = ::recv(mSocket, (char *)pData, bufSize, 0);

	return bytesReceived;
}

//*****************************************************************************
bool VuWinsockTcpSocket::lookupAddress(const char *strAddress, VUUINT32 &ipAddr)
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
	if ( VU_SSCANF(strAddress, "%d.%d.%d.%d", &addr[0], &addr[1], &addr[2], &addr[3]) == 4 )
	{
		ipAddr = (addr[0]<<24) | (addr[1]<<16) | (addr[2]<<8) | (addr[3]<<0);
		return true;
	}

	return false;
}
