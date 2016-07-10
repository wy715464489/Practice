#ifndef GATEWAY_GATEWAY_H_
#define GATEWAY_GATEWAY_H_

#include <map>
#include <vector>
#include <string>
#include "common/noncopyable.h"
#include "net/message.h"
#include "net/tcp_connection.h"
#include "net/tcp_server.h"

namespace gateway {

using net::MESSAGE_HEADER_ID;
using net::Message;
using net::MessageQueue;
using net::CloseCallback;
using net::TcpConnectionPtr;
using net::TcpServer;
using net::EventLoop;
using net::InetAddress;
using net::MessageHandler;

// Makesure id always larger than min_id
uint32_t AdjustId(uint32_t id, uint32_t min_id);

// client id always be kClientPeerId
// server id between [1, kMinClientPeerIdReserved)
// internal auto assigned client id [kMinClientPeerIdReserved, UNSIGNED_INT_MAX)
const uint32_t kClientPeerId = 0;
const uint32_t kMinClientPeerIdReserved = 1000000;

// Warning: not thread safe
// ID Range in (kMaxPeerIdReserved, max uint32_t]
uint32_t GeneratePeerId();

// Internal peer id => connection id
typedef std::map<MESSAGE_HEADER_ID, uint64_t> PEER_ID_TO_CONN_ID;

// Connectin id => internal peer id
typedef std::map<uint64_t, MESSAGE_HEADER_ID> CONN_ID_TO_PEER_ID;

// Connection id => last active time
typedef std::map<uint64_t, time_t> CLIENT_ACTIVE_TIMES;

// connection ids
typedef std::vector<uint64_t> IDLE_CLIENTS;

// Handle message to Gateway
class GatewayMessageHandler {
 public:
  GatewayMessageHandler() {}
  ~GatewayMessageHandler() {}

  bool handle_message(const Message& message, MessageQueue* output_messages);
  void remove(const TcpConnectionPtr& conn);
  void remove_by_id(uint64_t conn_id);
  void check_keepalive(time_t min_active_time,
                      IDLE_CLIENTS* idle_clients) const;

  // for unittest only, to check the internal status
  const CONN_ID_TO_PEER_ID& conn_id_to_peer_id() const {
    return conn_id_to_peer_id_;
  }
  const PEER_ID_TO_CONN_ID& peer_id_to_conn_id() const {
    return peer_id_to_conn_id_;
  }
  CLIENT_ACTIVE_TIMES& client_active_times() {
    return client_active_times_;
  }

 private:
  CONN_ID_TO_PEER_ID conn_id_to_peer_id_;
  PEER_ID_TO_CONN_ID peer_id_to_conn_id_;
  CLIENT_ACTIVE_TIMES client_active_times_;
  DISALLOW_COPY_AND_ASSIGN(GatewayMessageHandler);
};

class Gateway: public TcpServer {
 public:
  Gateway(EventLoop* loop,
          const InetAddress& listen_addr,
          MessageHandler message_handler,
          CloseCallback conn_close_callback,
          const GatewayMessageHandler& gateway_message_handler);

  // max_client_idle_time is in seconds
  void check_keepalive(int max_client_idle_time);

 private:
  // Connection closed
  virtual void connection_closed(const TcpConnectionPtr& conn);


  CloseCallback conn_close_callback_;
  const GatewayMessageHandler& gateway_message_handler_;
  DISALLOW_COPY_AND_ASSIGN(Gateway);
};

}  // namespace gateway
#endif  // GATEWAY_GATEWAY_H_
