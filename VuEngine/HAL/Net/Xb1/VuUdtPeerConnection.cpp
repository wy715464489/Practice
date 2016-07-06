#include "VuUdtPeerConnection.h"


//*****************************************************************************
VuUdtPeerConnection::VuUdtPeerConnection()
{
	mPort = VU_UDT_CONNECTION_PORT_DEFAULT;
	mLocalSocket = 0;
	mEPollId = 0;
}

//*****************************************************************************
int VuUdtPeerConnection::init()
{
	addrinfo hints;
	addrinfo* res;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;

	// Looks up local address and creates an addrinfo called 'res' we'll need 
	// to deallocate when we're done with it
	if (0 != getaddrinfo(NULL, mPort.c_str(), &hints, &res))
	{
		int err = WSAGetLastError();

		VUPRINTF("illegal port number or port is busy. (%0.8x)\n", err);
		
		return 0;
	}

	mLocalSocket = UDT::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	// Set our socket to be non-blocking in both directions
	bool off = false;
	UDT::setsockopt(mLocalSocket, 0, UDT_SNDSYN, &off, sizeof(off));
	UDT::setsockopt(mLocalSocket, 0, UDT_RCVSYN, &off, sizeof(off));

	// Winsock performance tweak of MTU
	int mss = 1052;
	UDT::setsockopt(mLocalSocket, 0, UDT_MSS, &mss, sizeof(int));

	// Bind our socket locally so we can receive messages
	if (UDT::ERROR == UDT::bind(mLocalSocket, res->ai_addr, (int)res->ai_addrlen))
	{
		VUPRINTF("bind: %s\n", UDT::getlasterror().getErrorMessage());
		return 0;
	}

	// Deallocate the addrinfo pointer that we got from getaddrinfo() above
	freeaddrinfo(res);

	if (UDT::listen(mLocalSocket, 10) != 0)
	{
		VUPRINTF("listen: %s\n", UDT::getlasterror().getErrorMessage());
	}

	VUPRINTF("Listening on port: %s\n", mPort.c_str());

	mEPollId = UDT::epoll_create();
	if (mEPollId == 0)
	{
		VUPRINTF("epoll_create: %s\n", UDT::getlasterror().getErrorMessage());
	}

	// Add our local listening socket to the poll list
	UDT::epoll_add_usock(mEPollId, mLocalSocket);

	return mLocalSocket;
}

//*****************************************************************************
void VuUdtPeerConnection::release()
{
	UDT::close(mLocalSocket);

	for (UDTSOCKET sock : mPeers)
	{
		UDT::close(sock);
	}
}

//*****************************************************************************
UDTSOCKET VuUdtPeerConnection::connect(SOCKADDR_STORAGE& socketAddress)
{
	UDTSOCKET newSocket = UDT::socket(socketAddress.ss_family, SOCK_DGRAM, IPPROTO_UDP);

	bool off = false;
	UDT::setsockopt(newSocket, 0, UDT_SNDSYN, &off, sizeof(off));
	UDT::setsockopt(newSocket, 0, UDT_RCVSYN, &off, sizeof(off));

	// connect to the server, implict bind
	if (UDT::ERROR == UDT::connect(newSocket, (sockaddr*)&socketAddress, sizeof(sockaddr_in6)))
	{
		VUPRINTF("connect: %s\n", UDT::getlasterror().getErrorMessage());
		return -1;
	}

	UDT::epoll_add_usock(mEPollId, newSocket);

	return newSocket;
}

//*****************************************************************************
int VuUdtPeerConnection::sendMessage(UDTSOCKET whichSocket, int sendType, void* pData, int dataSize)
{
	if (whichSocket == 0)
	{
		whichSocket = mLocalSocket;
	}

	UDTSTATUS status = UDT::getsockstate(whichSocket);
	if (status == UDTSTATUS::CONNECTED)
	{
		int ttl = -1;
		bool inorder = false;
		int result = UDT::sendmsg(whichSocket, (const char *)pData, dataSize, ttl, inorder);
		if (result == UDT::ERROR)
		{
			// Couldn't send the message
			VUPRINTF("UDT::sendmsg: %s\n", UDT::getlasterror().getErrorMessage());

			return -1;
		}
		else if (result != dataSize)
		{
			// Couldn't write the WHOLE message
			VUPRINTF("UDT::sendmsg couldn't write whole message: %s\n", UDT::getlasterror().getErrorMessage());

			return -1;
		}
		return result;
	}

	return 0;
}

//*****************************************************************************
void VuUdtPeerConnection::tick(float dT)
{
try
{
	std::set<UDTSOCKET> readFds;
	std::set<UDTSOCKET> writeFds;

	// Poll the sockets to see if there's any action
	if (UDT::epoll_wait(mEPollId, &readFds, &writeFds, 0, NULL, NULL) > 0)
	{
		// We have sockets we need to act upon
		for (UDTSOCKET readSocket : readFds)
		{
			UDTSTATUS status = UDT::getsockstate(readSocket);

			if (status == UDTSTATUS::CONNECTED)
			{
				int bytesRead = UDT::recvmsg(readSocket, mBuffer, VU_UDT_MAX_BUFFER_SIZE);
				if (bytesRead > 0)
				{
					// Copy data and Dispatch here
					for (int i = 0; i < bytesRead; i++)
					{
						putchar(mBuffer[i]);
					}
					//VUPRINTF("\n");
				}
			}
			else if (status == UDTSTATUS::LISTENING)
			{
				SOCKADDR_STORAGE clientaddr;
				int addrlen = sizeof(clientaddr);

				UDTSOCKET remoteSocket = UDT::accept(mLocalSocket, (sockaddr*)&clientaddr, &addrlen);
				if (remoteSocket != UDT::INVALID_SOCK)
				{
					std::string hostString;
					getSockAddrInfo((sockaddr*)&clientaddr, hostString);

					VUPRINTF("New Connection from %s (socket=%0.8x)\n", hostString.c_str(), remoteSocket);

					// Accept a new connection
					if (UDT::epoll_add_usock(mEPollId, remoteSocket, NULL) != 0)
					{
						VUPRINTF("epoll_add_usock: %s\n", UDT::getlasterror().getErrorMessage());
					}
					else
					{
						// Add to our list of peers
						mPeers.push_back(remoteSocket);
					}
				}
			}
		}

		// Anything to do for write sockets?
		for (UDTSOCKET writeSocket : writeFds)
		{
			UDTSTATUS status = UDT::getsockstate(writeSocket);
			if (status == UDTSTATUS::CONNECTED)
			{

			}
		}
	}
}
catch (...)
{
	VUPRINTF("EXCEPTION!\n");
}

}

void VuUdtPeerConnection::getSockAddrInfo(sockaddr* pSockAddr, std::string& outHost)
{

	if (pSockAddr->sa_family == AF_INET)
	{
		char addrStr[INET_ADDRSTRLEN];

		sockaddr_in* pAddr = (sockaddr_in*)(pSockAddr);
		inet_ntop(AF_INET, &(pAddr->sin_addr), addrStr, INET_ADDRSTRLEN);

		outHost = addrStr;
	}
	else if (pSockAddr->sa_family == AF_INET6)
	{
		char addrStr[INET6_ADDRSTRLEN];

		sockaddr_in6* pAddr = (sockaddr_in6*)(pSockAddr);
		inet_ntop(AF_INET6, &(pAddr->sin6_addr), addrStr, INET6_ADDRSTRLEN);

		outHost = addrStr;
	}
	else
	{
		// Unknown protocol domain/family
		VUPRINTF("unknown AF_* family type: %d\n", pSockAddr->sa_family);
	}
}
