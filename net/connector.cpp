#include "net/connector.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <tr1/functional>
#include "net/socket_ops.h"
#include "net/eventloop.h"

namespace net {

Connector::Connector(EventLoop* loop, const InetAddress& server_addr)
  : _loop(loop),
    _server_addr(server_addr) {
}

Connector::~Connector() {
  if (_channel != NULL) {
    _channel->disable_all();
  }
}

void Connector::connect() {
  // Please check man connect's EINPROGRESS
  const int fd = CreateNonblockingSocketOrDie();
  ConnectStatus status = Connect(fd, _server_addr.addr());
  if (status == kConnectContinue) {
    connecting(fd);
  } else if (status == kConnectRetry) {
    retry(fd);
  } else {
    // ERROR_LOG("Fatal to connect server %s:%u, reason=%s\n",
    //           server_addr_.ip().c_str(),
    //           server_addr_.port(),
    //           strerror(errno));
    // TODO(Weitong): can we do better than just exit immediately?
    exit(1);
  }
}

void Connector::set_new_connection_callback(const NewConnectionCallback& cb) {
  _new_connection_callback = cb;
}

void Connector::connecting(int fd) {
  _channel.reset(new Channel(_loop->poller(), fd));
  // set channel tie
  _channel->tie(shared_from_this());
  _channel->set_write_callback(std::tr1::bind(&Connector::handle_write, this));
  _channel->set_close_callback(std::tr1::bind(&Connector::handle_write, this));
  _channel->set_error_callback(std::tr1::bind(&Connector::handle_write, this));
  _channel->enable_writing();
}

void Connector::retry(int fd) {
  close(fd);
  // Should retry in 1, 2, 4, 8, 16 ..seconds?
  _loop->run_after(1000,
        std::tr1::bind(&Connector::connect, shared_from_this()));
}

void Connector::handle_write() {
  _channel->disable_all();
  const int fd = _channel->fd();
  const int error = GetSocketError(fd);
  if (error) {
    // ERROR_LOG("Failed to connect server %s:%u reason:%s\n",
    //           server_addr_.ip().c_str(), server_addr_.port(),
    //           strerror(errno));
    retry(fd);
  } else {
    _new_connection_callback(fd, _server_addr);
  }
}

}