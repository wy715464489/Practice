// Copyright [2012-2014] <HRG>
#ifndef NET_TCP_CONNECTION_H_
#define NET_TCP_CONNECTION_H_

#include <stdint.h>
#include <tr1/memory>
#include <tr1/functional>
#include <map>
#include <string>
#include "net/inet_address.h"
#include "net/buffer.h"
#include "net/message.h"
#include "common/noncopyable.h"

 namespace net {

class Channel;
class Poller;
class Socket;
class Buffer;

typedef std::tr1::function<void (const TcpConnectionPtr&)> CloseCallback;

// the data has been read to (buf, len)
typedef std::tr1::function<void (const TcpConnectionPtr&,
                              Buffer*)> DataCallback;

// For Acceptor and Connector
typedef std::tr1::function<void (int fd, const InetAddress&)>
          NewConnectionCallback;

typedef std::map<uint64_t, TcpConnectionPtr > ConnectionMap;

class TcpConnection: public std::tr1::enable_shared_from_this<TcpConnection> {
 public:
  // Active connections = (opened -closed)
  static uint64_t gTcpConnectionOpened;
  static uint64_t gTcpConnectionClosed;
  // Closed by read or write error
  static uint64_t gTcpConnectionErrorClosed;
  // Data received and sent (in Bytes)
  static uint64_t gTcpConnectionDataReceived;
  static uint64_t gTcpConnectionDataSent;
  // Message received and sent
  static uint64_t gTcpConnectionMessageReceived;
  static uint64_t gTcpConnectionMessageSent;

  TcpConnection(std::tr1::shared_ptr<Poller> poller,
                int fd,  // socket fd
                const InetAddress& peer_addr);
  ~TcpConnection();

  // **must** be called after constructor immediately
  // since shared_from_this() can not be called in constructor
  void tie_channel();

  void set_close_callback(const CloseCallback& cb);
  void set_data_callback(const DataCallback& cb);

  // Connection id
  uint64_t id() const;
  uint32_t  peer_ip() const;

  // Be invoked by client or server
  bool send(const char* data, int len);

  // Be invoked by client or server
  void handle_close();

  void shrink_buffer();

  bool output_empty() const;   

 private:
  void handle_read();
  void handle_write();
  void handle_error();

  std::tr1::shared_ptr<Socket>  socket_;
  std::tr1::shared_ptr<Channel> channel_;
  uint64_t id_;  // Connection's global id
  int32_t  peer_ip_;   // peer_ip
  InetAddress peer_addr_;  // peer_addr
  CloseCallback close_callback_;
  DataCallback  data_callback_;
  Buffer input_buffer_;
  Buffer output_buffer_;
  DISALLOW_COPY_AND_ASSIGN(TcpConnection);
};

void DefaultConnectionDataCallback(MessageHandler message_handler,
                                   TcpConnectionPtr src_conn,
                                   const ConnectionMap& dst_conns,
                                   Buffer* buffer);

}  // namespace net


#endif  // NET_TCP_CONNECTION_H_
