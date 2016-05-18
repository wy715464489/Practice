#include "net/tcp_connection.h"
#include <unistd.h>
#include <fcntl.h>
#include <tr1/functional>
#include <stdio.h>
#include <string>
#include <sys/time.h>
#include "net/socket.h"
#include "net/poller.h"
#include "net/channel.h"
#include "net/socket_ops.h"

namespace net {	

uint64_t TcpConnection::gTcpConnectionOpened = 0;
uint64_t TcpConnection::gTcpConnectionClosed = 0;
uint64_t TcpConnection::gTcpConnectionErrorClosed = 0;
uint64_t TcpConnection::gTcpConnectionDataReceived = 0;
uint64_t TcpConnection::gTcpConnectionDataSent = 0;
uint64_t TcpConnection::gTcpConnectionMessageReceived = 0;
uint64_t TcpConnection::gTcpConnectionMessageSent = 0;

TcpConnection::TcpConnection(std::tr1::shared_ptr<Poller> poller,
                             int fd,
                             const InetAddress& peer_addr)
	: _socket(new Socket(fd)),
		_channel(new Channel(poller, fd)),
		_id(++gTcpConnectionOpened), //start from 1
		_peer_addr(peer_addr) {
	// set channel attributes
	_channel->set_read_callback(
      std::tr1::bind(&TcpConnection::handle_read, this));
  _channel->set_write_callback(
      std::tr1::bind(&TcpConnection::handle_write, this));
  _channel->set_close_callback(
      std::tr1::bind(&TcpConnection::handle_close, this));
  _channel->set_error_callback(
      std::tr1::bind(&TcpConnection::handle_error, this));
  _channel->enable_reading();

 //  // set socket attributes
  _socket->set_nonblock();
  _socket->set_keepalive();
  _socket->set_tcpnodelay();

  _peer_ip = AddressToNetwork(_peer_addr.ip());
}

TcpConnection::~TcpConnection() {
	++gTcpConnectionClosed;
	_channel->disable_all();
}

void TcpConnection::tie_channel() {
	_channel->tie(shared_from_this());
}

uint64_t TcpConnection::id() const {
  return _id;
}

uint32_t TcpConnection::peer_ip() const {
  return _peer_ip;
}

void TcpConnection::set_close_callback(const CloseCallback& cb) {
  _close_callback = cb;
}

void TcpConnection::set_data_callback(const DataCallback& cb) {
  _data_callback = cb;
}

bool TcpConnection::send(const char* data, int len) {
	struct timeval begin, end;
  gettimeofday(&begin, NULL);

  // If nothing in output queue, try writing directly
  if (!_channel->is_writing() && _output_buffer.readable_bytes() == 0) {
    // As similar as apache apr library
    int rv;
    int count = 0;
    do {
      rv = write(_channel->fd(), data, len);
      ++count;
    } while (rv == -1 && errno == EINTR);

  
    gettimeofday(&end, NULL);
    uint64_t elapsed_in_us = (end.tv_sec - begin.tv_sec) * 1000000
                       + (end.tv_usec - begin.tv_usec);
    

    if (rv >= 0) {
      // Bytes have been sent
      gTcpConnectionDataSent += rv;
      // Maybe only particial data has been sent
      const int remaining = len - rv;
      if (remaining > 0) {
        _output_buffer.append(data + rv, remaining);
        _channel->enable_writing();

        gettimeofday(&end, NULL);
        uint64_t elapsed_in_us = (end.tv_sec - begin.tv_sec) * 1000000
                           + (end.tv_usec - begin.tv_usec);
        
      }
    } else if (errno != EAGAIN) {
      handle_error();

      
      return false;
    } else {
      const int remaining = len;
      if (remaining > 0) {
        _output_buffer.append(data, remaining);
        _channel->enable_writing();

        gettimeofday(&end, NULL);
        uint64_t elapsed_in_us = (end.tv_sec - begin.tv_sec) * 1000000
                           + (end.tv_usec - begin.tv_usec);
        
      }
    }
  } else {
    _output_buffer.append(data, len);

    gettimeofday(&end, NULL);
    uint64_t elapsed_in_us = (end.tv_sec - begin.tv_sec) * 1000000
                       + (end.tv_usec - begin.tv_usec);
   
  }

  return true;
}

void TcpConnection::handle_read() {
	int rv = _input_buffer.read_socket(_channel->fd());
  if (rv > 0) {
    // Bytes have been recevied
    gTcpConnectionDataReceived += rv;
    _data_callback(shared_from_this(), &_input_buffer);
  } else if (rv == 0) {
    handle_close();
  } else if (errno != EAGAIN) {
    handle_error();
  }
}

void TcpConnection::handle_write() {
	// if (!channel_->is_writing()) {
 //    ERROR_LOG("In TcpConnection::handle_write, channel status "
 //              "must be writing, fd=%d\n", socket_->fd());
 //  }

  // As similar as apache apr library
  int rv;
  do {
    rv = write(_channel->fd(), _output_buffer.read_begin(),
              _output_buffer.readable_bytes());
  } while (rv == -1 && errno == EINTR);

  if (rv >= 0) {
    // Bytes have been sent
    gTcpConnectionDataSent += rv;
    _output_buffer.retrieve(rv);
    if (_output_buffer.readable_bytes() == 0) {
      _channel->disable_writing();
    }
  } else if (errno != EAGAIN) {
    handle_error();
  }
}

void TcpConnection::handle_close() {
 	_channel->disable_all();
  _close_callback(shared_from_this());
}

void TcpConnection::handle_error() {
	_channel->disable_all();
  ++gTcpConnectionErrorClosed;
  _close_callback(shared_from_this());
}

void TcpConnection::shrink_buffer() {
	_input_buffer.shrink();
  _output_buffer.shrink();
}

bool TcpConnection::output_empty() const {
	return _output_buffer.readable_bytes() == 0;
}

void DefaultConnectionDataCallback(MessageHandler message_handler,
                                   TcpConnectionPtr src_conn,
                                   const ConnectionMap& dst_conns,
                                   Buffer* buffer) {
  //int message_handled = 0;
  // TODO(Weitong): Connection should has limited message processed every time
  while (buffer->readable_bytes() >= kMessageHeaderLength) {
    const struct MessageHeader* message_header =
          reinterpret_cast<const struct MessageHeader*>(buffer->read_begin());
    if (message_header->length() < kMessageHeaderLength) {
      // log error message and close the connection
      // ERROR_LOG("message header length error, length: %d, type:%d, src_id: %d, dst_id: %d\n",
      //            message_header->length(), message_header->type,
      //            message_header->src_id, message_header->dst_id);
      src_conn->handle_close();
      break;
    }

    // printf("message_header->length:%d, type:%d", message_header->length(), message_header->type);

    if (message_header->length() > buffer->readable_bytes()) {
      // log error message and close the connection
      // DEBUG_LOG("Wait more data to compose a full message, full length:%d, type:%d, src_id:%d, dst_id:%d, current_length:%lu\n",
		    // message_header->length(), message_header->type,
		    // message_header->src_id, message_header->dst_id,
		    // buffer->readable_bytes());
      break;
    }

    /** 进入了buffer的消息必须得一次性处理完，如果后续没有消息进来，
       DefaultConnectionDataCallback不会被唤醒
       TODO(Weitong): 是否可以优化的方法解决?
    if (++message_handled >= kMaxMessageHanledEachCallBack) {
      printf("message overflow:%d\n", message_handled);
      break;
    }
    */
    ++TcpConnection::gTcpConnectionMessageReceived;

    const char* message_body = reinterpret_cast<const char*>(
              buffer->read_begin() + kMessageHeaderLength);
    const size_t message_body_length =
              message_header->length() - kMessageHeaderLength;

    // TODO(Weitong): verify checksum
    // if failed, call tcp connection closed
    buffer->retrieve(message_header->length());

    Message message(*message_header, message_body, message_body_length);
    message.set_owner(src_conn->id());
    message.set_src_ip(src_conn->peer_ip());

    MessageQueue outputs;
    if (!message_handler(message, &outputs)) {
      src_conn->handle_close();
      break;
    }

    // messages may be sent fail, but the percentage must be very small
    TcpConnection::gTcpConnectionMessageSent += outputs.size();

    for (MessageQueue::const_iterator it = outputs.begin();
        it != outputs.end(); ++it) {
      std::tr1::shared_ptr<Message> message = *it;
      ConnectionMap::const_iterator dst_conn =
                                    dst_conns.find(message->owner());
      if (dst_conn != dst_conns.end()) {
        // may send fail, so we hold the dst conn first
        TcpConnectionPtr conn = dst_conn->second;
        // only valid conn can be put in dst_conns,
        // just in case somebody invalid this rule
        if (conn != NULL) {
          conn->send(message->data(), message->data_length());
        } else {
          // ERROR_LOG("send message error, type:%u, function:%s\n",
          //     message->header().type, __FUNCTION__);
        }
      } else {
        // ERROR_LOG("dest conn can not be found by conn id:%lu\n",
        //     message->owner());
      }
    }
  }
}
}