// Copyright [2012-2014] <HRG>
#include "net/socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "net/socket_ops.h"
#include "net/inet_address.h"
#include "common/log.h"
using common::LogSystem;
using common::ERROR_LOG;

 namespace net {

Socket::Socket(int fd):fd_(fd) {
}

Socket::~Socket() {
  ::close(fd_);
}

int Socket::fd() const {
  return fd_;
}

void Socket::bind(const InetAddress& addr) {
  BindOrDie(fd_, addr.addr());
}

void Socket::listen() {
  ListenOrDie(fd_);
}

int Socket::accept(InetAddress* peer_addr) {
  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  socklen_t addrlen = sizeof(struct sockaddr);

  int fd = ::accept(fd_, reinterpret_cast<struct sockaddr*>(&addr), &addrlen);
  if (fd >= 0) {
      peer_addr->set_addr(addr);
  }
  return fd;
}

void Socket::set_reuse_addr() {
  int optval = 1;
  int rv = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof optval));
  if (rv < 0) {
    ERROR_LOG("Failed to set_reuse_addr for fd=%d, reason=%s\n",
                fd_, strerror(errno));
  }
}

void Socket::set_nonblock() {
  int flags = ::fcntl(fd_, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int rv = ::fcntl(fd_, F_SETFL, flags);
  if (rv < 0) {
    ERROR_LOG("Failed to set_nonblock for fd=%d, reason=%s\n",
                fd_, strerror(errno));
  }
}

void Socket::set_tcpnodelay() {
  int optval = 1;
  int rv = ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof optval));
  if (rv < 0) {
    ERROR_LOG("Failed to set_tcpnodelay for fd=%d, reason=%s\n",
                fd_, strerror(errno));
  }
}

void Socket::set_keepalive() {
  int optval = 1;
  int rv = ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE,
               &optval, static_cast<socklen_t>(sizeof optval));
  if (rv < 0) {
    ERROR_LOG("Failed to set_keepalive for fd=%d, reason=%s\n",
                fd_, strerror(errno));
  }
}

}  // namespace net

