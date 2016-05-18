#ifndef NET_TCP_CLIENT_H_
#define NET_TCP_CLIENT_H_

#include <tr1/functional>
#include <tr1/memory>
#include "inet_address.h"
#include "net/tcp_connection.h"

namespace net {
	class EventLoop;
	class Connector;

class TcpClient {
 public:
 	TcpClient(EventLoop* loop,
            const InetAddress& server_addr,
            MessageHandler message_handler);
  virtual ~TcpClient();

  // Asynchronous connect
  void connect();

  // Return false if failed
  bool send(const char* data, int len);

  bool output_empty() const;

  void shrink_connection_buffer();
 private:
 	// Send server register message, log additional info ..etc
  virtual void connection_opened(const TcpConnectionPtr& conn);

  // Connection new data arrives
  virtual void connection_new_data(const TcpConnectionPtr& conn,
                                      Buffer* buffer);

  void new_connection(int fd, const InetAddress& inet_addr);


  void reconnect(const TcpConnectionPtr& conn);
 	
	EventLoop* _loop;
	InetAddress _server_addr;
	MessageHandler _message_handler;
	std::tr1::shared_ptr<Connector> _connector;
	std::tr1::shared_ptr<TcpConnection> _conn;
	ConnectionMap _connections;
	DISALLOW_COPY_AND_ASSIGN(TcpClient);
};
}

#endif  // NET_TCP_CLIENT_H_