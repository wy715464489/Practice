// Copyright [2012-2014] <HRG>
#ifndef NET_TCP_SERVER_H_
#define NET_TCP_SERVER_H_

#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include "net/tcp_connection.h"
#include "net/acceptor.h"
class EventLoop;
class InetAddress;

 namespace net {

class TcpServer {
 public:
  TcpServer(EventLoop* loop,
            const InetAddress& listen_addr,
            MessageHandler message_handler);
  virtual ~TcpServer();

  void start();

  void shrink_connection_buffer();

 protected:
  // Close connection
  void close_connection(const TcpConnectionPtr& conn);

  void close_connection_by_id(uint64_t conn_id);


 private:
  // Connection new data arrives
  virtual void connection_new_data(const TcpConnectionPtr& conn,
                                      Buffer* buffer);
  // New connection arrives, for accept
  void new_connection(int fd, const InetAddress& inet_addr);

  EventLoop* loop_;
  ConnectionMap connections_;
  std::tr1::shared_ptr<Acceptor> acceptor_;
  MessageHandler  message_handler_;
  const InetAddress& server_addr_;
  DISALLOW_COPY_AND_ASSIGN(TcpServer);
};

}  // namespace net


#endif  // NET_TCP_SERVER_H_
