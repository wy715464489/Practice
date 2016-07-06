#pragma once

#include <list>

#include <WinSock2.h>

// Default port for all connections
#define VU_UDP_CONNECTION_PORT_DEFAULT "34343"

// Maximum number of sockets we'll hold onto and query the epoll mechanism for
#define VU_UDP_MAX_FDS 16

// Our temporary read buffer, how big is it?
#define VU_UDP_MAX_BUFFER_SIZE	(32 * 1024)

struct VuPacket
{
	void*		mData;
	int			mDataLen;
	VUINT32		mAddrHash;
};

class VuUdpPeerConnection
{
public:
	typedef std::queue<VuPacket> PacketList;

	VuUdpPeerConnection();

	SOCKET			init();
	void			release();

	SOCKET			connect(SOCKADDR_STORAGE& socketAddress);
	int				disconnect();
	
	int				sendMessage(const SOCKADDR_STORAGE* pStorage, void* pData, int dataSize);
	void			tick(float dt);
	int				readPacket(void *pData, int bufferSize, int* bytesRead, VUUINT32* addrHash);

private:
	void					addToPacketList(SOCKADDR_STORAGE* pStorage, void* pData, int dataSize);

	void					getSockAddrInfo(sockaddr* pSockAddr, std::string& outHost);

	char*					getLastErrorStr();	// Last socket error reported by socket system

	SOCKET					mLocalSocket;
	std::string				mPort;
	int						mEPollId;
	char					mBuffer[VU_UDP_MAX_BUFFER_SIZE];
	PacketList				mPacketList;

	char					mLastErrorStr[_MAX_PATH];
};
