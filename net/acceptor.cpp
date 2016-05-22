#include "net/acceptor.h"
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "net/eventloop.h"

namespace net {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_addr)
  : _loop(loop),
    _socket(CreateNonblockingSocketOrDie()),
    _channel(new Channel(loop->poller(), _socket.fd())),
    _idle_fd(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    _socket.set_reuse_addr();
    _socket.bind(listen_addr);
    _channel->set_read_callback(
                    std::tr1::bind(&Acceptor::handle_read, this));
}

Acceptor::~Acceptor() {
  _channel->disable_all();
}

void Acceptor::handle_read() {
  // Loop until no more fd in backup queue
  while (1) {
    InetAddress peer_addr;
    int fd = _socket.accept(&peer_addr);
    if (fd >= 0) {
      _new_connection_callback(fd, peer_addr);
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
      if (errno == EMFILE) {
        ::close(_idle_fd);
        _idle_fd = ::accept(_socket.fd(), NULL, NULL);
        ::close(_idle_fd);
        _idle_fd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
      } else {
        // TODO(Weitong): can we do better than just exit immediately?
        exit(1);
      }
    }
  }
}

void Acceptor::set_new_connection_callback(const NewConnectionCallback& cb) {
  _new_connection_callback = cb;
}

}