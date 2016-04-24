#include "socket_ops.h"
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

namespace net {

void IgnorePipeSignal() {
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	// In order to make SIGPIPE ignore always effective
  	// does not use signal(SIGPIPE, SIG_IGN)
  	sigaction(SIGPIPE, &sa, 0);
}

int CreateNonblockingSocketOrDie() {
	int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(fd < 0)
	{
		exit(1);
	}

	int flags = ::fcntl(fd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	int rv = ::fcntl(fd, F_GETFL, flags);
	if(rv < 0) {
		exit(1);
	}
	
	return fd;
}

void BindOrDie(int fd, const struct sockaddr_in& addr) {
	int rv = ::bind(fd, reinterpret_cast<const struct sockaddr*>(&addr),
						sizeof(struct sockaddr));
	if(rv < 0) {
		exit(1);
	}
}

void ListenOrDie(int fd) {
	int rv = ::listen(fd, 511);
	if(rv < 0) {
		exit(1);
	}
}

ConnectStatus Connect(int fd, const struct sockaddr_in& addr) {
	int rv = ::connect(fd, reinterpret_cast<const struct sockaddr*>(&addr),
						sizeof(struct sockaddr));
	if(rv == 0) {
		return kConnectContinue;
	}

	switch(errno) {
		case EINPROGRESS:
  	case EINTR:
  	case EISCONN:
    	return kConnectContinue;
  	case EAGAIN:
  	case EADDRINUSE:
  	case EADDRNOTAVAIL:
  	case ECONNREFUSED:
  	case ENETUNREACH:
    	return kConnectRetry;
  	default:
    return kConnectFatalError;
	}
}

int GetSocketError(int fd) {
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof optval);

	if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

uint32_t AddressToNetwork(const std::string& ip) {
	return inet_network(ip.c_str());
}

const char* NetworkToAddress(uint32_t ip) {
	in_addr addr;
	addr.s_addr = htonl(ip);
	return inet_ntoa(addr);
}

bool IsInternalIp(uint32_t ip) {
	if ((ip >= 0x0A000000 && ip <= 0x0AFFFFFF)
      ||(ip >= 0xAC100000 && ip <= 0xAC1FFFFF)
      ||(ip >= 0xC0A80000 && ip <= 0xC0A8FFFF)
      || ip == 0x7F000001) {
    return true;
  } else {
    return false;
  }
}

}