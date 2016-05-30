// Copyright [2012-2014] <HRG>
#include "net/connector.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <tr1/functional>
#include "net/socket_ops.h"
#include "net/eventloop.h"
#include "common/log.h"
using hrg::common::LogSystem;
using hrg::common::ERROR_LOG;

namespace hrg { namespace net {

Connector::Connector(EventLoop* loop, const InetAddress& server_addr)
  : loop_(loop),
    server_addr_(server_addr) {
}

Connector::~Connector() {
  if (channel_ != NULL) {
    channel_->disable_all();
  }
}

void Connector::connect() {
  // Please check man connect's EINPROGRESS
  const int fd = CreateNonblockingSocketOrDie();
  ConnectStatus status = Connect(fd, server_addr_.addr());
  if (status == kConnectContinue) {
    connecting(fd);
  } else if (status == kConnectRetry) {
    retry(fd);
  } else {
    ERROR_LOG("Fatal to connect server %s:%u, reason=%s\n",
              server_addr_.ip().c_str(),
              server_addr_.port(),
              strerror(errno));
    // TODO(Weitong): can we do better than just exit immediately?
    exit(1);
  }
}

void Connector::set_new_connection_callback(const NewConnectionCallback& cb) {
  new_connection_callback_ = cb;
}

void Connector::connecting(int fd) {
  channel_.reset(new Channel(loop_->poller(), fd));
  // set channel tie
  channel_->tie(shared_from_this());
  channel_->set_write_callback(std::tr1::bind(&Connector::handle_write, this));
  channel_->set_close_callback(std::tr1::bind(&Connector::handle_write, this));
  channel_->set_error_callback(std::tr1::bind(&Connector::handle_write, this));
  channel_->enable_writing();
}

void Connector::retry(int fd) {
  close(fd);
  // Should retry in 1, 2, 4, 8, 16 ..seconds?
  loop_->run_after(1000,
        std::tr1::bind(&Connector::connect, shared_from_this()));
}

void Connector::handle_write() {
  channel_->disable_all();
  const int fd = channel_->fd();
  const int error = GetSocketError(fd);
  if (error) {
    ERROR_LOG("Failed to connect server %s:%u reason:%s\n",
              server_addr_.ip().c_str(), server_addr_.port(),
              strerror(errno));
    retry(fd);
  } else {
    new_connection_callback_(fd, server_addr_);
  }
}

}  // namespace net
}  // namespace hrg
