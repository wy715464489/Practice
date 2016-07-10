// Copyright [2012-2014] <HRG>
#include "net/socket_ops.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "common/log.h"
using common::LogSystem;
using common::ERROR_LOG;

 namespace net {

void IgnorePipeSignal() {
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  // In order to make SIGPIPE ignore always effective
  // does not use signal(SIGPIPE, SIG_IGN)
  sigaction(SIGPIPE, &sa, 0);
}

int CreateNonblockingSocketOrDie() {
  int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd < 0) {
    ERROR_LOG("Create socket failed, reason:%s\n", strerror(errno));
    exit(1);
  }

  int flags = ::fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int rv = ::fcntl(fd, F_SETFL, flags);
  if (rv < 0) {
    ERROR_LOG("Failed to set_nonblock for fd=%d, reason=%s\n",
                fd, strerror(errno));
    exit(1);
  }
  return fd;
}

void BindOrDie(int fd, const struct sockaddr_in& addr) {
  int rv = ::bind(fd, reinterpret_cast<const struct sockaddr*>(&addr),
              sizeof(struct sockaddr));
  if (rv < 0) {
    ERROR_LOG("Bind socket failed, addr:%s, port:%u, reason:%s\n",
              inet_ntoa(addr.sin_addr),
              ntohs(addr.sin_port),
              strerror(errno));
    fprintf(stderr, "Bind socket failed");  // For unittest test only
    exit(1);
  }
}

void ListenOrDie(int fd) {
  int rv = ::listen(fd, 511);  // This backlog is large enough
  if (rv < 0) {
    ERROR_LOG("Listen failed, fd=%d, reason:%s\n", fd, strerror(errno));
    fprintf(stderr, "Listen failed");  // For unittest test only
    exit(1);
  }
}

ConnectStatus Connect(int fd, const struct sockaddr_in& addr) {
  int rv = ::connect(fd, reinterpret_cast<const struct sockaddr*>(&addr),
              sizeof(struct sockaddr));
  if (rv == 0) {
    return kConnectContinue;
  }

  switch (errno) {
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

}  // namespace net

