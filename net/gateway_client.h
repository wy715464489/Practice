// Copyright [2012-2014] <HRG>
#ifndef NET_GATEWAY_CLIENT_H_
#define NET_GATEWAY_CLIENT_H_

#include "net/tcp_connection.h"
#include "net/tcp_client.h"
#include "net/inet_address.h"
using net::TcpClient;
using net::MessageHandler;
using net::TcpConnectionPtr;

namespace net {

class GatewayClient: public TcpClient {
 public:
  GatewayClient(net::EventLoop* loop,
                const net::InetAddress& gateway_addr,
                MessageHandler message_handler,
                int global_id);
  virtual ~GatewayClient();

 private:
  // Connection opened
  virtual void connection_opened(const TcpConnectionPtr& conn);
  int global_id_;
};

}  // namespace net
#endif  // NET_GATEWAY_CLIENT_H_
