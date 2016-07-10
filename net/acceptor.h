// Copyright [2012-2014] <HRG>
#ifndef NET_ACCEPTOR_H_
#define NET_ACCEPTOR_H_

#include <tr1/functional>
#include "net/inet_address.h"
#include "net/socket_ops.h"
#include "net/socket.h"
#include "net/channel.h"
#include "net/tcp_connection.h"
#include "common/noncopyable.h"

 namespace net {

class EventLoop;
class Acceptor: public std::tr1::enable_shared_from_this<Acceptor> {
 public:
  Acceptor(EventLoop* loop, const InetAddress& listen_addr);
  ~Acceptor();

  void listen();

  void handle_read();

  void set_new_connection_callback(const NewConnectionCallback& cb);

 private:
  EventLoop* loop_;
  Socket socket_;
  std::tr1::shared_ptr<Channel> channel_;
  int idle_fd_;  // when errno == EMFILE
  NewConnectionCallback new_connection_callback_;
  DISALLOW_COPY_AND_ASSIGN(Acceptor);
};

}  // namespace net

#endif  // NET_ACCEPTOR_H_
