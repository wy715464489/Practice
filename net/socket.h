// Copyright [2012-2014] <HRG>
#ifndef NET_SOCKET_H_
#define NET_SOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "common/noncopyable.h"

namespace hrg { namespace net {

class InetAddress;

class Socket {
 public:
  explicit Socket(int fd);
  ~Socket();

  int fd() const;

  // Die if failed
  void bind(const InetAddress& addr);

  // Die if faile
  void connect(const InetAddress& addr);

  // Die if failed
  void listen();

  // On succcess, a non-negative integer return and peer_addr is set
  // On error, -1 is returned, and errno  is  set  appropriately.
  int accept(InetAddress* peer_addr);

  void set_reuse_addr();
  void set_nonblock();
  void set_tcpnodelay();
  void set_keepalive();

 private:
  const int fd_;
  DISALLOW_COPY_AND_ASSIGN(Socket);
};

}  // namespace net
}  // namespace hrg

#endif  // NET_SOCKET_H_
