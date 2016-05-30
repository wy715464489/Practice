// Copyright [2012-2014] <HRG>
#include "net/acceptor.h"
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "common/log.h"
#include "net/eventloop.h"
using hrg::common::LogSystem;
using hrg::common::ERROR_LOG;

namespace hrg { namespace net {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_addr)
  : loop_(loop),
    socket_(CreateNonblockingSocketOrDie()),
    channel_(new Channel(loop->poller(), socket_.fd())),
    idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    socket_.set_reuse_addr();
    socket_.bind(listen_addr);
    channel_->set_read_callback(
                    std::tr1::bind(&Acceptor::handle_read, this));
}

Acceptor::~Acceptor() {
  channel_->disable_all();
}

void Acceptor::listen() {
  socket_.listen();
  // set channel tie
  channel_->tie(shared_from_this());
  channel_->enable_reading();
}

void Acceptor::handle_read() {
  // Loop until no more fd in backup queue
  while (1) {
    InetAddress peer_addr;
    int fd = socket_.accept(&peer_addr);
    if (fd >= 0) {
      new_connection_callback_(fd, peer_addr);
    } else if (errno == EAGAIN) {
      break;
    } else if (errno == EINTR || errno == ECONNABORTED) {
      // Can call accept again immediately
    } else {
      // http://search.cpan.org/~mlehmann/EV-4.17/libev/ev.pod
      // #The_special_problem_of_accept()ing_when_you_can't
      // The last way to handle it is to simply log the error and exit,
      // as is often done with malloc failures,
      // but this results in an easy opportunity for a DoS attack.
      ERROR_LOG("Accept fatal error:%s\n", strerror(errno));
      if (errno == EMFILE) {
        ::close(idle_fd_);
        idle_fd_ = ::accept(socket_.fd(), NULL, NULL);
        ::close(idle_fd_);
        idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
      } else {
        // TODO(Weitong): can we do better than just exit immediately?
        exit(1);
      }
    }
  }
}

void Acceptor::set_new_connection_callback(const NewConnectionCallback& cb) {
  new_connection_callback_ = cb;
}

}  // namespace net
}  // namespace hrg
