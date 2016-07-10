// Copyright [2012-2014] <HRG>
#include "net/tcp_client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <map>
#include "net/socket_ops.h"
#include "net/eventloop.h"
#include "net/connector.h"
#include "common/log.h"

 namespace net {

using common::LogSystem;
using common::INFO_LOG;
using common::ERROR_LOG;

TcpClient::TcpClient(EventLoop* loop,
                    const InetAddress& server_addr,
                    MessageHandler message_handler)
    :loop_(loop),
     server_addr_(server_addr),
     message_handler_(message_handler),
     connector_(new Connector(loop, server_addr)) {
  connector_->set_new_connection_callback(std::tr1::bind(
                                    &TcpClient::new_connection,
                                    this,
                                    std::tr1::placeholders::_1,
                                    std::tr1::placeholders::_2));
}

TcpClient::~TcpClient() {
}

void TcpClient::connect() {
  INFO_LOG("TcpClient: try to connect server %s:%u\n",
            server_addr_.ip().c_str(), server_addr_.port());
  connector_->connect();
}

bool TcpClient::send(const char* data, int len) {
  if (conn_ != NULL) {
    return conn_->send(data, len);
  } else {
    return false;
  }
}

bool TcpClient::output_empty() const {
  if (conn_ != NULL) {
    return conn_->output_empty();
  }
  return true;
}

void TcpClient::connection_opened(const TcpConnectionPtr& conn) {
  printf("TcpClient: connected server %s:%u, conn_id:%lu\n",
            server_addr_.ip().c_str(), server_addr_.port(),
            conn->id());
}

void TcpClient::connection_new_data(const TcpConnectionPtr& conn,
                                      Buffer* buffer) {
  DefaultConnectionDataCallback(message_handler_, conn, connections_, buffer);
}


void TcpClient::new_connection(int fd, const InetAddress& inet_addr) {
  INFO_LOG("TcpClient: connected server %s:%u\n",
            server_addr_.ip().c_str(), server_addr_.port());
  TcpConnectionPtr conn(new TcpConnection(loop_->poller(),
                                      fd,
                                      inet_addr));
  conn->tie_channel();
  // Auto reconnect when connection was closed
  conn->set_close_callback(std::tr1::bind(&TcpClient::reconnect,
                                          this,
                                          std::tr1::placeholders::_1));
  conn->set_data_callback(std::tr1::bind(&TcpClient::connection_new_data,
                                        this,
                                        std::tr1::placeholders::_1,
                                        std::tr1::placeholders::_2));
  connections_.insert(std::make_pair(conn->id(), conn));
  conn_ = conn;
  connection_opened(conn_);
}

void TcpClient::reconnect(const TcpConnectionPtr& conn) {
  ERROR_LOG("TcpClient disconnected from server %s:%u, try to reconnect\n",
            server_addr_.ip().c_str(), server_addr_.port());
  connections_.erase(conn->id());
  conn_.reset();
  connect();
}

void TcpClient::shrink_connection_buffer() {
  for (ConnectionMap::iterator it = connections_.begin();
      it != connections_.end();
      ++it) {
    it->second->shrink_buffer();
  }
}

}  // namespace net

