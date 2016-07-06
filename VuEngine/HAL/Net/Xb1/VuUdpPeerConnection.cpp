
#include "VuUdpPeerConnection.h"

#include "VuEngine/Util/VuHash.h"

//*****************************************************************************
VuUdpPeerConnection::VuUdpPeerConnection()
{
	mPort = VU_UDP_CONNECTION_PORT_DEFAULT;
	mLocalSocket = 0;
	mEPollId = 0;
}

//*****************************************************************************
SOCKET VuUdpPeerConnection::init()
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
		VUPRINTF("getaddrinfo(): %s\n", getLastErrorStr());
		return INVALID_SOCKET;
	}

	mLocalSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (mLocalSocket == INVALID_SOCKET)
	{
		VUPRINTF("socket(): %s\n", getLastErrorStr());
		return INVALID_SOCKET;
	}

	u_long mode = 1; // non-blocking operation
	if (NO_ERROR != ioctlsocket(mLocalSocket, FIONBIO, &mode))
	{
		VUPRINTF("ioctlsocket(): %s\n", getLastErrorStr());
		return INVALID_SOCKET;
	}

	//bool off = false;
	//if (SOCKET_ERROR == setsockopt(mLocalSocket, 0, SO_KEEPALIVE, (char*)&off, sizeof(off)))
	//{
	//	VUPRINTF("setsockopt(): %s\n", getLastErrorStr());
	//	return INVALID_SOCKET;
	//}

	// Winsock performance tweak of MTU
	//int mss = 1052;
	//UDT::setsockopt(mLocalSocket, 0, UDT_MSS, &mss, sizeof(int));

	// Bind our socket locally so we can receive messages
	if (SOCKET_ERROR == bind(mLocalSocket, res->ai_addr, (int)res->ai_addrlen))
	{
		VUPRINTF("bind(): %s\n", getLastErrorStr());
		return INVALID_SOCKET;
	}

	// Deallocate the addrinfo pointer that we got from getaddrinfo() above
	freeaddrinfo(res);

	//if (SOCKET_ERROR == listen(mLocalSocket, 10))
	//{
	//	VUPRINTF("listen(): %s\n", getLastErrorStr());
	//	return INVALID_SOCKET;
	//}

	//VUPRINTF("Listening on port: %s\n", mPort.c_str());


	//mEPollId = UDT::epoll_create();
	//if (mEPollId == 0)
	//{
	//	VUPRINTF("epoll_create: %s\n", getLastErrorStr());
	//}

	//// Add our local listening socket to the poll list
	//UDT::epoll_add_usock(mEPollId, mLocalSocket);

	return mLocalSocket;
}

//*****************************************************************************
void VuUdpPeerConnection::release()
{
	closesocket(mLocalSocket);
}

//*****************************************************************************
SOCKET VuUdpPeerConnection::connect(SOCKADDR_STORAGE& socketAddress)
{
	SOCKET newSocket = socket(socketAddress.ss_family, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == newSocket)
	{
		VUPRINTF("socket(): %s\n", getLastErrorStr());
		return INVALID_SOCKET;
	}

	//bool off = false;
	//UDT::setsockopt(newSocket, 0, UDT_SNDSYN, &off, sizeof(off));
	//UDT::setsockopt(newSocket, 0, UDT_RCVSYN, &off, sizeof(off));

	u_long mode = 1; // non-blocking operation
	if (NO_ERROR != ioctlsocket(newSocket, FIONBIO, &mode))
	{
		VUPRINTF("ioctlsocket(): %s\n", getLastErrorStr());
		return INVALID_SOCKET;
	}

	// connect to the server, implict bind
	if (SOCKET_ERROR == ::connect(newSocket, (sockaddr*)&socketAddress, sizeof(sockaddr_in6)))
	{
		VUPRINTF("connect(): %s\n", getLastErrorStr());
		return INVALID_SOCKET;
	}

//	UDT::epoll_add_usock(mEPollId, newSocket);

	return newSocket;
}

//*****************************************************************************
int VuUdpPeerConnection::sendMessage(const SOCKADDR_STORAGE* pStorage, void* pData, int dataSize)
{

	int result = sendto(mLocalSocket, (const char *)pData, dataSize, 0, (sockaddr*)pStorage, sizeof(SOCKADDR_STORAGE));
	if (SOCKET_ERROR == result)
	{
		// Couldn't send the message
		VUPRINTF("send(): %s\n", getLastErrorStr());

		return -1;
	}
	else if (result != dataSize)
	{
		// Couldn't write the WHOLE message
		VUPRINTF("send(): couldn't write whole message: %s\n", getLastErrorStr());

		return -1;
	}

	return result;
}

//*****************************************************************************
void VuUdpPeerConnection::tick(float dT)
{
	fd_set readFds;
	fd_set writeFds;
	fd_set exceptFds;
	
	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&exceptFds);

	timeval timeout;
	timeout.tv_sec = 0L;
	timeout.tv_usec = 0L;

	FD_SET(mLocalSocket, &readFds);

	int selectResult = select(0, &readFds, &writeFds, &exceptFds, &timeout);
	if (selectResult == SOCKET_ERROR)
	{
		VUPRINTF("select(): %s\n", getLastErrorStr());
	}

	// We have sockets we need to act upon
	if (FD_ISSET(mLocalSocket, &readFds))
	{
		SOCKADDR_STORAGE remoteSockAddr;
		int remoteSockAddrSize = sizeof(remoteSockAddr);

		ZeroMemory(&remoteSockAddr, remoteSockAddrSize);

		int bytesRead = recvfrom(mLocalSocket, mBuffer, VU_UDP_MAX_BUFFER_SIZE, 0, (sockaddr*)&remoteSockAddr, &remoteSockAddrSize);

		if (bytesRead > 0)
		{
			addToPacketList(&remoteSockAddr, (void*)mBuffer, bytesRead);
		}
	}
		
	// Anything to do for write sockets?
	if (FD_ISSET(mLocalSocket, &writeFds))
	{
	}

	// Anything to do for exception sockets?
	if (FD_ISSET(mLocalSocket, &exceptFds))
	{
	}
}

//*****************************************************************************
int VuUdpPeerConnection::readPacket(void *pData, int bufferSize, int* bytesRead, VUUINT32* addrHash)
{
	if (mPacketList.size() > 0)
	{
		// Work to do
		VuPacket packet = mPacketList.front();
		mPacketList.pop();

		memcpy_s(pData, bufferSize, packet.mData, packet.mDataLen);
		*bytesRead = packet.mDataLen;
		*addrHash = packet.mAddrHash;
		
		return packet.mDataLen;
	}

	return 0;
}

// Private

//*****************************************************************************
void VuUdpPeerConnection::addToPacketList(SOCKADDR_STORAGE* pStorage, void* pData, int dataLen)
{
	VuPacket packet;

	VUUINT32 remoteHash = VuHash::crc32((void *)pStorage, sizeof(SOCKADDR_STORAGE));

	packet.mAddrHash = remoteHash;
	packet.mData = malloc((size_t)dataLen);
	packet.mDataLen = dataLen;

	memcpy_s(packet.mData, dataLen, pData, dataLen);

	mPacketList.push(packet);
}

//*****************************************************************************
void VuUdpPeerConnection::getSockAddrInfo(sockaddr* pSockAddr, std::string& outHost)
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

//*****************************************************************************
char* VuUdpPeerConnection::getLastErrorStr()
{
	int err = WSAGetLastError();

	switch (err)
	{
		case WSAEINTR: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEINTR: A blocking operation was interrupted by a call to WSACancelBlockingCall.", _TRUNCATE); break;
		case WSAEBADF: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEBADF: The file handle supplied is not valid.", _TRUNCATE); break;
		case WSAEACCES: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEACCES: An attempt was made to access a socket in a way forbidden by its access permissions.", _TRUNCATE); break;
		case WSAEFAULT: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEFAULT: The system detected an invalid pointer address in attempting to use a pointer argument in a call.", _TRUNCATE); break;
		case WSAEINVAL: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEINVAL: An invalid argument was supplied.", _TRUNCATE); break;
		case WSAEMFILE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEMFILE: Too many open sockets.", _TRUNCATE); break;
		case WSAEWOULDBLOCK: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEWOULDBLOCK: A non-blocking socket operation could not be completed immediately.", _TRUNCATE); break;
		case WSAEINPROGRESS: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEINPROGRESS: A blocking operation is currently executing.", _TRUNCATE); break;
		case WSAEALREADY: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEALREADY: An operation was attempted on a non-blocking socket that already had an operation in progress.", _TRUNCATE); break;
		case WSAENOTSOCK: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAENOTSOCK: An operation was attempted on something that is not a socket.", _TRUNCATE); break;
		case WSAEDESTADDRREQ: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEDESTADDRREQ: A required address was omitted from an operation on a socket.", _TRUNCATE); break;
		case WSAEMSGSIZE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEMSGSIZE: A message sent on a datagram socket was larger than the internal message buffer or some other network limit, or the buffer used to receive a datagram into was smaller than the datagram itself.", _TRUNCATE); break;
		case WSAEPROTOTYPE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEPROTOTYPE: A protocol was specified in the socket function call that does not support the semantics of the socket type requested.", _TRUNCATE); break;
		case WSAENOPROTOOPT: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAENOPROTOOPT: An unknown, invalid, or unsupported option or level was specified in a getsockopt or setsockopt call.", _TRUNCATE); break;
		case WSAEPROTONOSUPPORT: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEPROTONOSUPPORT: The requested protocol has not been configured into the system, or no implementation for it exists.", _TRUNCATE); break;
		case WSAESOCKTNOSUPPORT: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAESOCKTNOSUPPORT: The support for the specified socket type does not exist in this address family.", _TRUNCATE); break;
		case WSAEOPNOTSUPP: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEOPNOTSUPP: The attempted operation is not supported for the type of object referenced.", _TRUNCATE); break;
		case WSAEPFNOSUPPORT: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEPFNOSUPPORT: The protocol family has not been configured into the system or no implementation for it exists.", _TRUNCATE); break;
		case WSAEAFNOSUPPORT: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEAFNOSUPPORT: An address incompatible with the requested protocol was used.", _TRUNCATE); break;
		case WSAEADDRINUSE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEADDRINUSE: Only one usage of each socket address (protocol/network address/port) is normally permitted.", _TRUNCATE); break;
		case WSAEADDRNOTAVAIL: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEADDRNOTAVAIL: The requested address is not valid in its context.", _TRUNCATE); break;
		case WSAENETDOWN: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAENETDOWN: A socket operation encountered a dead network.", _TRUNCATE); break;
		case WSAENETUNREACH: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAENETUNREACH: A socket operation was attempted to an unreachable network.", _TRUNCATE); break;
		case WSAENETRESET: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAENETRESET: The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress.", _TRUNCATE); break;
		case WSAECONNABORTED: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAECONNABORTED: An established connection was aborted by the software in your host machine.", _TRUNCATE); break;
		case WSAECONNRESET: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAECONNRESET: An existing connection was forcibly closed by the remote host.", _TRUNCATE); break;
		case WSAENOBUFS: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAENOBUFS: An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full.", _TRUNCATE); break;
		case WSAEISCONN: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEISCONN: A connect request was made on an already connected socket.", _TRUNCATE); break;
		case WSAENOTCONN: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAENOTCONN: A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using a sendto call) no address was supplied.", _TRUNCATE); break;
		case WSAESHUTDOWN: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAESHUTDOWN: A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown call.", _TRUNCATE); break;
		case WSAETOOMANYREFS: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAETOOMANYREFS: Too many references to some kernel object.", _TRUNCATE); break;
		case WSAETIMEDOUT: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAETIMEDOUT: A connection attempt failed because the connected party did not properly respond after a period of time, or established connection failed because connected host has failed to respond.", _TRUNCATE); break;
		case WSAECONNREFUSED: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAECONNREFUSED: No connection could be made because the target machine actively refused it.", _TRUNCATE); break;
		case WSAELOOP: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAELOOP: Cannot translate name.", _TRUNCATE); break;
		case WSAENAMETOOLONG: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAENAMETOOLONG: Name component or name was too long.", _TRUNCATE); break;
		case WSAEHOSTDOWN: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEHOSTDOWN: A socket operation failed because the destination host was down.", _TRUNCATE); break;
		case WSAEHOSTUNREACH: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEHOSTUNREACH: A socket operation was attempted to an unreachable host.", _TRUNCATE); break;
		case WSAENOTEMPTY: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAENOTEMPTY: Cannot remove a directory that is not empty.", _TRUNCATE); break;
		case WSAEPROCLIM: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEPROCLIM: A Windows Sockets implementation may have a limit on the number of applications that may use it simultaneously.", _TRUNCATE); break;
		case WSAEUSERS: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEUSERS: Ran out of quota.", _TRUNCATE); break;
		case WSAEDQUOT: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEDQUOT: Ran out of disk quota.", _TRUNCATE); break;
		case WSAESTALE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAESTALE: File handle reference is no longer available.", _TRUNCATE); break;
		case WSAEREMOTE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEREMOTE: Item is not available locally.", _TRUNCATE); break;
		case WSASYSNOTREADY: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSASYSNOTREADY: WSAStartup cannot function at this time because the underlying system it uses to provide network services is currently unavailable.", _TRUNCATE); break;
		case WSAVERNOTSUPPORTED: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAVERNOTSUPPORTED: The Windows Sockets version requested is not supported.", _TRUNCATE); break;
		case WSANOTINITIALISED: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSANOTINITIALISED: Either the application has not called WSAStartup, or WSAStartup failed.", _TRUNCATE); break;
		case WSAEDISCON: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEDISCON: Returned by WSARecv or WSARecvFrom to indicate the remote party has initiated a graceful shutdown sequence.", _TRUNCATE); break;
		case WSAENOMORE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAENOMORE: No more results can be returned by WSALookupServiceNext.", _TRUNCATE); break;
		case WSAECANCELLED: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAECANCELLED: A call to WSALookupServiceEnd was made while this call was still processing. The call has been canceled.", _TRUNCATE); break;
		case WSAEINVALIDPROCTABLE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEINVALIDPROCTABLE: The procedure call table is invalid.", _TRUNCATE); break;
		case WSAEINVALIDPROVIDER: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEINVALIDPROVIDER: The requested service provider is invalid.", _TRUNCATE); break;
		case WSAEPROVIDERFAILEDINIT: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEPROVIDERFAILEDINIT: The requested service provider could not be loaded or initialized.", _TRUNCATE); break;
		case WSASYSCALLFAILURE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSASYSCALLFAILURE: A system call has failed.", _TRUNCATE); break;
		case WSASERVICE_NOT_FOUND: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSASERVICE_NOT_FOUND: No such service is known. The service cannot be found in the specified name space.", _TRUNCATE); break;
		case WSATYPE_NOT_FOUND: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSATYPE_NOT_FOUND: The specified class was not found.", _TRUNCATE); break;
		case WSA_E_NO_MORE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_E_NO_MORE: No more results can be returned by WSALookupServiceNext.", _TRUNCATE); break;
		case WSA_E_CANCELLED: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_E_CANCELLED: A call to WSALookupServiceEnd was made while this call was still processing. The call has been canceled.", _TRUNCATE); break;
		case WSAEREFUSED: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAEREFUSED: A database query failed because it was actively refused.", _TRUNCATE); break;
		case WSAHOST_NOT_FOUND: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSAHOST_NOT_FOUND: No such host is known.", _TRUNCATE); break;
		case WSATRY_AGAIN: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSATRY_AGAIN: This is usually a temporary error during hostname resolution and means that the local server did not receive a response from an authoritative server.", _TRUNCATE); break;
		case WSANO_RECOVERY: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSANO_RECOVERY: A non-recoverable error occurred during a database lookup.", _TRUNCATE); break;
		case WSANO_DATA: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSANO_DATA: The requested name is valid, but no data of the requested type was found.", _TRUNCATE); break;
		case WSA_QOS_RECEIVERS: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_RECEIVERS: At least one reserve has arrived.", _TRUNCATE); break;
		case WSA_QOS_SENDERS: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_SENDERS: At least one path has arrived.", _TRUNCATE); break;
		case WSA_QOS_NO_SENDERS: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_NO_SENDERS: There are no senders.", _TRUNCATE); break;
		case WSA_QOS_NO_RECEIVERS: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_NO_RECEIVERS: There are no receivers.", _TRUNCATE); break;
		case WSA_QOS_REQUEST_CONFIRMED: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_REQUEST_CONFIRMED: Reserve has been confirmed.", _TRUNCATE); break;
		case WSA_QOS_ADMISSION_FAILURE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_ADMISSION_FAILURE: Error due to lack of resources.", _TRUNCATE); break;
		case WSA_QOS_POLICY_FAILURE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_POLICY_FAILURE: Rejected for administrative reasons - bad credentials.", _TRUNCATE); break;
		case WSA_QOS_BAD_STYLE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_BAD_STYLE: Unknown or conflicting style.", _TRUNCATE); break;
		case WSA_QOS_BAD_OBJECT: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_BAD_OBJECT: Problem with some part of the filterspec or providerspecific buffer in general.", _TRUNCATE); break;
		case WSA_QOS_TRAFFIC_CTRL_ERROR: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_TRAFFIC_CTRL_ERROR: Problem with some part of the flowspec.", _TRUNCATE); break;
		case WSA_QOS_GENERIC_ERROR: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_GENERIC_ERROR: General QOS error.", _TRUNCATE); break;
		case WSA_QOS_ESERVICETYPE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_ESERVICETYPE: An invalid or unrecognized service type was found in the flowspec.", _TRUNCATE); break;
		case WSA_QOS_EFLOWSPEC: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_EFLOWSPEC: An invalid or inconsistent flowspec was found in the QOS structure.", _TRUNCATE); break;
		case WSA_QOS_EPROVSPECBUF: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_EPROVSPECBUF: Invalid QOS provider-specific buffer.", _TRUNCATE); break;
		case WSA_QOS_EFILTERSTYLE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_EFILTERSTYLE: An invalid QOS filter style was used.", _TRUNCATE); break;
		case WSA_QOS_EFILTERTYPE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_EFILTERTYPE: An invalid QOS filter type was used.", _TRUNCATE); break;
		case WSA_QOS_EFILTERCOUNT: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_EFILTERCOUNT: An incorrect number of QOS FILTERSPECs were specified in the FLOWDESCRIPTOR.", _TRUNCATE); break;
		case WSA_QOS_EOBJLENGTH: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_EOBJLENGTH: An object with an invalid ObjectLength field was specified in the QOS provider-specific buffer.", _TRUNCATE); break;
		case WSA_QOS_EFLOWCOUNT: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_EFLOWCOUNT: An incorrect number of flow descriptors was specified in the QOS structure.", _TRUNCATE); break;
		case WSA_QOS_EUNKOWNPSOBJ: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_EUNKOWNPSOBJ: An unrecognized object was found in the QOS provider-specific buffer.", _TRUNCATE); break;
		case WSA_QOS_EPOLICYOBJ: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_EPOLICYOBJ: An invalid policy object was found in the QOS provider-specific buffer.", _TRUNCATE); break;
		case WSA_QOS_EFLOWDESC: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_EFLOWDESC: An invalid QOS flow descriptor was found in the flow descriptor list.", _TRUNCATE); break;
		case WSA_QOS_EPSFLOWSPEC: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_EPSFLOWSPEC: An invalid or inconsistent flowspec was found in the QOS provider specific buffer.", _TRUNCATE); break;
		case WSA_QOS_EPSFILTERSPEC: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_EPSFILTERSPEC: An invalid FILTERSPEC was found in the QOS provider-specific buffer.", _TRUNCATE); break;
		case WSA_QOS_ESDMODEOBJ: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_ESDMODEOBJ: An invalid shape discard mode object was found in the QOS provider specific buffer.", _TRUNCATE); break;
		case WSA_QOS_ESHAPERATEOBJ: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_ESHAPERATEOBJ: An invalid shaping rate object was found in the QOS provider-specific buffer.", _TRUNCATE); break;
		case WSA_QOS_RESERVED_PETYPE: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_QOS_RESERVED_PETYPE: A reserved policy element was found in the QOS provider-specific buffer.", _TRUNCATE); break;
		case WSA_SECURE_HOST_NOT_FOUND: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_SECURE_HOST_NOT_FOUND: No such host is known securely.", _TRUNCATE); break;
		case WSA_IPSEC_NAME_POLICY_ERROR: VU_STRNCPY(mLastErrorStr, _MAX_PATH, "WSA_IPSEC_NAME_POLICY_ERROR: Name based IPSEC policy could not be added.", _TRUNCATE); break;

		default:
			VU_SNPRINTF(mLastErrorStr, _MAX_PATH, _TRUNCATE, "Unknown WinSock Error (%#.0x)", err);
	}

	return mLastErrorStr;
}
