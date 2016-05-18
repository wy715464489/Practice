#ifndef NET_SOCKET_H_
#define NET_SOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include "common/noncopyable.h"

namespace net {

class InetAddress;

class Socket {
 public:
 	explicit Socket(int fd);
 	~Socket();

 	int fd() const;

 	void bind(const InetAddress& addr);

 	void connect(const InetAddress& addr);

 	void listen();

 	int accept(InetAddress* peer_addr);

 	void set_reuse_addr();
  void set_nonblock();
  void set_tcpnodelay();
  void set_keepalive();
private:
  const int _fd;
  DISALLOW_COPY_AND_ASSIGN(Socket);
};

}

#endif  // NET_SOCKET_H_