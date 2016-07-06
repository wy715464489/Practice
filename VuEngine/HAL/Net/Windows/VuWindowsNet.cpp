//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Interface class to Network library.
// 
//*****************************************************************************

#include <ppltasks.h>
#include "VuEngine/HAL/Net/VuNet.h"
#include "VuEngine/Util/VuUtf8.h"

using namespace concurrency;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuNet, VuNet);


class VuWindowsTcpSocket : public VuTcpSocket
{
public:
	virtual bool		setNonBlocking(bool nonBlocking);
	virtual bool		setTimeOut(int receiveMS, int sendMS);
	virtual bool		listen(int backlog);
	virtual VuTcpSocket	*accept();
	virtual bool		connect(const char *hostName, int port, int timeoutMS);
	virtual int			send(const void *pData, int dataSize);
	virtual int			recv(void *pData, int bufSize);

	StreamSocket		^mSocket;
};


//*****************************************************************************
VuTcpSocket *VuTcpSocket::create(VUUINT16 port)
{
	return VUNULL;
	//return new VuWindowsTcpSocket;
}

//*****************************************************************************
bool VuWindowsTcpSocket::setNonBlocking(bool nonBlocking)
{
	return true;
}

//*****************************************************************************
bool VuWindowsTcpSocket::setTimeOut(int receiveMS, int sendMS)
{
	return true;
}

//*****************************************************************************
bool VuWindowsTcpSocket::listen(int backlog)
{
	VUASSERT(0, "Not implemented on Windows");
	return VUNULL;
}

//*****************************************************************************
VuTcpSocket *VuWindowsTcpSocket::accept()
{
	VUASSERT(0, "Not implemented on Windows");
	return VUNULL;
}

//*****************************************************************************
bool VuWindowsTcpSocket::connect(const char *hostName, int port, int timeoutMS)
{
	mSocket = ref new StreamSocket();

	std::wstring wstrHostName;
	VuUtf8::convertUtf8StringToWCharString(hostName, wstrHostName);
	HostName^ nativeHostName = ref new HostName(ref new Platform::String(wstrHostName.c_str()));

	wchar_t wstrPort[16];
	swprintf_s(wstrPort, 16, L"%d", port);
	Platform::String ^nativePort = ref new Platform::String(wstrPort);

	create_task(mSocket->ConnectAsync(nativeHostName, nativePort, SocketProtectionLevel::PlainSocket)).then([this] (task<void> previousTask) 
	{ 
		try 
		{ 
			// Try getting all exceptions from the continuation chain above this point. 
			int test0 = 0;
			previousTask.get(); 
			int test1 = 0;
		} 
		catch (Platform::Exception^ exception) 
		{ 
			// failed to connect
			int test = 0;
		} 
	});

	return true;
}

//*****************************************************************************
int VuWindowsTcpSocket::send(const void *pData, int dataSize)
{
	return dataSize;
}

//*****************************************************************************
int VuWindowsTcpSocket::recv(void *pData, int bufSize)
{
	return 0;
}
