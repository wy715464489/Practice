#ifndef NET_CONNECTOR_H_
#define NET_CONNECTOR_H_

#include <tr1/functional>
#include "inet_address.h"
#include "socket_ops.h"
#include "socket.h"
#include "channel.h"
#include "net/tcp_connection.h"
#include "common/noncopyable.h"

namespace net {
class EventLoop;

class Connector: public std::tr1::enable_shared_from_this<Connector> {
 public:
  Connector(EventLoop* loop, const InetAddress& server_addr);
  ~Connector();

  void connect();

  void set_new_connection_callback(const NewConnectionCallback& cb);

 private:
  void connecting(int fd);
  void retry(int fd);
  void handle_write();

  EventLoop* _loop;
  InetAddress _server_addr;
  std::tr1::shared_ptr<Channel> _channel;
  NewConnectionCallback _new_connection_callback;
  DISALLOW_COPY_AND_ASSIGN(Connector);
};
}

#endif