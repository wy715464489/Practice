#include "socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "socket_ops.h"
#include "net/inet_address.h"

namespace net {

Socket::Socket(int fd):_fd(fd) {
}

Socket::~Socket() {
  ::close(_fd);	
}

int Socket::fd() const {
  return _fd;
}

void Socket::bind(const InetAddress& addr) {
  BindOrDie(_fd, addr.addr());
}

void Socket::listen() {
  ListenOrDie(_fd);
}

int Socket::accept(InetAddress* peer_addr) {
  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  socklen_t addrlen = sizeof(struct sockaddr);

  int fd = ::accept(_fd, reinterpret_cast<struct sockaddr*>(&addr), &addrlen);
  if (fd >= 0) {
      peer_addr->set_addr(addr);
  }
  return fd;
}

void Socket::set_reuse_addr() {
  int optval = 1;
  int rv = ::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::set_nonblock() {
  int flags = ::fcntl(_fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int rv = ::fcntl(_fd, F_SETFL, flags);
  // if (rv < 0) {
  //   ERROR_LOG("Failed to set_nonblock for fd=%d, reason=%s\n",
  //               fd_, strerror(errno));
  // }
}

void Socket::set_tcpnodelay() {
  int optval = 1;
  int rv = ::setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof optval));
  // if (rv < 0) {
  //   ERROR_LOG("Failed to set_tcpnodelay for fd=%d, reason=%s\n",
  //               fd_, strerror(errno));
  // }
}

void Socket::set_keepalive() {
  int optval = 1;
  int rv = ::setsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE,
               &optval, static_cast<socklen_t>(sizeof optval));
  // if (rv < 0) {
  //   ERROR_LOG("Failed to set_keepalive for fd=%d, reason=%s\n",
  //               fd_, strerror(errno));
  // }
}

}