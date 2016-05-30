// Copyright [2012-2014] <HRG>
#ifndef NET_CONNECTOR_H_
#define NET_CONNECTOR_H_

#include <tr1/functional>
#include "net/inet_address.h"
#include "net/socket_ops.h"
#include "net/socket.h"
#include "net/channel.h"
#include "net/tcp_connection.h"
#include "common/noncopyable.h"

namespace hrg { namespace net {

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

  EventLoop* loop_;
  InetAddress server_addr_;
  std::tr1::shared_ptr<Channel> channel_;
  NewConnectionCallback new_connection_callback_;
  DISALLOW_COPY_AND_ASSIGN(Connector);
};

}  // namespace net
}  // namespace hrg
#endif  // NET_CONNECTOR_H_
