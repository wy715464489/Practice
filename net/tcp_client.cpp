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

namespace net {

TcpClient::TcpClient(EventLoop* loop,
                    const InetAddress& server_addr,
                    MessageHandler message_handler)
    :_loop(loop),
     _server_addr(server_addr),
     _message_handler(message_handler),
     _connector(new Connector(loop, server_addr)) {
  _connector->set_new_connection_callback(std::tr1::bind(
                                    &TcpClient::new_connection,
                                    this,
                                    std::tr1::placeholders::_1,
                                    std::tr1::placeholders::_2));
}

TcpClient::~TcpClient() {
}

void TcpClient::connect() {
  _connector->connect();
}

bool TcpClient::send(const char* data, int len) {
  if (_conn != NULL) {
    return _conn->send(data, len);
  } else {
    return false;
  }
}

bool TcpClient::output_empty() const {
  if (_conn != NULL) {
    return _conn->output_empty();
  }
  return true;
}

void TcpClient::connection_opened(const TcpConnectionPtr& conn) {
  printf("TcpClient: connected server %s:%u, conn_id:%lu\n",
            _server_addr.ip().c_str(), _server_addr.port(),
            conn->id());
}

void TcpClient::connection_new_data(const TcpConnectionPtr& conn,
                                      Buffer* buffer) {
  DefaultConnectionDataCallback(_message_handler, conn, _connections, buffer);
}

void TcpClient::new_connection(int fd, const InetAddress& inet_addr) {
  TcpConnectionPtr conn(new TcpConnection(_loop->poller(),
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
  _connections.insert(std::make_pair(conn->id(), conn));
  _conn = conn;
  connection_opened(_conn);
}

void TcpClient::reconnect(const TcpConnectionPtr& conn) {
  _connections.erase(conn->id());
  _conn.reset();
  connect();
}

void TcpClient::shrink_connection_buffer() {
  for (ConnectionMap::iterator it = _connections.begin();
      it != _connections.end();
      ++it) {
    it->second->shrink_buffer();
  }
}

}