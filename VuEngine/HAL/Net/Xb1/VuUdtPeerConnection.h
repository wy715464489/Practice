#pragma once

#include <list>

#include <WinSock2.h>
#include <WS2tcpip.h>
//#include <WSPiApi.h>

#include "udt/udt.h"

// Default port for all connections
#define VU_UDT_CONNECTION_PORT_DEFAULT "34343"

// Maximum number of sockets we'll hold onto and query the epoll mechanism for
#define VU_UDT_MAX_FDS 16

// Our temporary read buffer, how big is it?
#define VU_UDT_MAX_BUFFER_SIZE	(32 * 1024)


class VuUdtPeerConnection
{
public:
	typedef std::vector<UDTSOCKET> PeerList;

	VuUdtPeerConnection();

	UDTSOCKET init();
	void release();
	UDTSOCKET connect(SOCKADDR_STORAGE& socketAddress);
	int disconnect(UDTSOCKET socket);
	int sendMessage(UDTSOCKET whichSocket, int sendType, void* pData, int dataSize);
	void tick(float dt);
	PeerList& getPeers() { return mPeers; }
	void getSockAddrInfo(sockaddr* pSockAddr, std::string& outHost);

private:
	UDTSOCKET				mLocalSocket;
	std::string				mPort;
	int						mEPollId;
	char					mBuffer[VU_UDT_MAX_BUFFER_SIZE];
	PeerList				mPeers;
};
