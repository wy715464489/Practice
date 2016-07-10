// Copyright [2012-2014] <HRG>
#include "net/tcp_server.h"
#include <stdio.h>
#include <string.h>
#include <tr1/functional>
#include <sstream>
#include <string>
#include "net/eventloop.h"
#include "net/inet_address.h"
#include "net/acceptor.h"
#include "common/log.h"

 namespace net {

using common::LogSystem;
using common::INFO_LOG;

TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listen_addr,
                     MessageHandler message_handler)
  : loop_(loop),
    acceptor_(new Acceptor(loop, listen_addr)),
    message_handler_(message_handler),
    server_addr_(listen_addr) {
}

TcpServer::~TcpServer() {
}

void TcpServer::start() {
  acceptor_->set_new_connection_callback(
      std::tr1::bind(&TcpServer::new_connection,
                this,
                std::tr1::placeholders::_1,
                std::tr1::placeholders::_2));
  acceptor_->listen();
}

void TcpServer::close_connection(const TcpConnectionPtr& conn) {
  close_connection_by_id(conn->id());
}

void TcpServer::close_connection_by_id(uint64_t conn_id) {
  ConnectionMap::const_iterator it = connections_.find(conn_id);
  if (it != connections_.end()) {
    const uint32_t peer_ip = it->second->peer_ip();
    connections_.erase(conn_id);
    INFO_LOG("TcpServer: client %s disconnected from TcpServer %s:%u\n",
            NetworkToAddress(peer_ip),
            server_addr_.ip().c_str(), server_addr_.port());
  }
}

void TcpServer::shrink_connection_buffer() {
  for (ConnectionMap::iterator it = connections_.begin();
      it != connections_.end();
      ++it) {
    it->second->shrink_buffer();
  }
}

void TcpServer::connection_new_data(const TcpConnectionPtr& conn,
                                      Buffer* buffer) {
  DefaultConnectionDataCallback(message_handler_, conn, connections_, buffer);
}

void TcpServer::new_connection(int fd, const InetAddress& inet_addr) {
  INFO_LOG("TcpServer: client %s connected TcpServer %s:%u\n",
            inet_addr.ip().c_str(),
            server_addr_.ip().c_str(), server_addr_.port());
  TcpConnectionPtr conn(new TcpConnection(loop_->poller(),
                                      fd,
                                      inet_addr));
  conn->tie_channel();
  conn->set_close_callback(std::tr1::bind(&TcpServer::close_connection,
                                          this,
                                          std::tr1::placeholders::_1));
  conn->set_data_callback(std::tr1::bind(&TcpServer::connection_new_data,
                                        this,
                                        std::tr1::placeholders::_1,
                                        std::tr1::placeholders::_2));
  connections_.insert(std::make_pair(conn->id(), conn));
}


}  // namespace net

