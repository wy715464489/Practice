#include "net/tcp_server.h"
#include <stdio.h>
#include <string.h>
#include <tr1/functional>
#include <sstream>
#include <string>
#include "net/eventloop.h"
#include "net/inet_address.h"
#include "net/acceptor.h"

namespace net {

TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listen_addr,
                     MessageHandler message_handler)
  : _loop(loop),
    _acceptor(new Acceptor(loop, listen_addr)),
    _message_handler(message_handler),
    _server_addr(listen_addr) {
}

TcpServer::~TcpServer() {
}

void TcpServer::start() {
  _acceptor->set_new_connection_callback(
      std::tr1::bind(&TcpServer::new_connection,
                this,
                std::tr1::placeholders::_1,
                std::tr1::placeholders::_2));
  _acceptor->listen();
}

void TcpServer::close_connection(const TcpConnectionPtr& conn) {
  close_connection_by_id(conn->id());
}

void TcpServer::close_connection_by_id(uint64_t conn_id) {
  ConnectionMap::const_iterator it = _connections.find(conn_id);
  if (it != _connections.end()) {
    //const uint32_t peer_ip = it->second->peer_ip();
    _connections.erase(conn_id);
  }
}

void TcpServer::shrink_connection_buffer() {
  for (ConnectionMap::iterator it = _connections.begin();
      it != _connections.end();
      ++it) {
    it->second->shrink_buffer();
  }
}

void TcpServer::connection_new_data(const TcpConnectionPtr& conn,
                                      Buffer* buffer) {
  DefaultConnectionDataCallback(_message_handler, conn, _connections, buffer);
}

void TcpServer::new_connection(int fd, const InetAddress& inet_addr) {
  TcpConnectionPtr conn(new TcpConnection(_loop->poller(),
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
  _connections.insert(std::make_pair(conn->id(), conn));
}

}
