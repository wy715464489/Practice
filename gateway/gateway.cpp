#include "gateway/gateway.h"
#include <tr1/memory>
#include "net/tcp_connection.h"
#include "net/socket_ops.h"
#include "common/log.h"
#include "proto/communication/common_enum.pb.h"

using net::NetworkToAddress;

namespace gateway {

using net::MESSAGE_HEADER_TYPE;
using net::MESSAGE_HEADER_ID;
using net::IsInternalIp;
using net::TcpConnection;
using std::make_pair;
using common::LogSystem;
using common::DEBUG_LOG;
using common::INFO_LOG;
using common::ERROR_LOG;

uint32_t AdjustId(uint32_t id, uint32_t min_id) {
  return (id <= min_id) ? min_id + 1 : id;
}

uint32_t GeneratePeerId() {
  static uint32_t next_peer_id = kMinClientPeerIdReserved;
  ++next_peer_id;
  next_peer_id = AdjustId(next_peer_id, kMinClientPeerIdReserved);
  return next_peer_id;
}

bool GatewayMessageHandler::handle_message(const Message& message,
                                           MessageQueue* output_messages) {
  MESSAGE_HEADER_TYPE type = message.header().type;
  DEBUG_LOG("message arrives: length:%u, type:%u,"
            "src_id=%u, dst_id=%u, owner_conn=%lu,"
            "src_ip=%s sequence:%u checksum:%u\n",
            message.header().length(),
            message.header().type,
            message.header().src_id,
            message.header().dst_id,
            message.owner(),
            NetworkToAddress(message.src_ip()),
            message.header().sequence(),
            message.header().checksum);

  MESSAGE_HEADER_ID src_id = message.header().src_id;
  MESSAGE_HEADER_ID dst_id = message.header().dst_id;
  uint64_t      owner_conn = message.owner();
  
  net::MessageHeader header;
  header.set_length(message.header().length());
  header.type = message.header().type;
  header.src_id = src_id;
  header.dst_id = dst_id;
  header.set_sequence(message.header().sequence());
  header.checksum = message.header().checksum;

  std::tr1::shared_ptr<Message> out_message(new Message(
                           header,
                           message.body(),
                           message.body_length()));
  out_message->set_owner(owner_conn);
  out_message->set_src_ip(message.src_ip());
  output_messages->push_back(out_message);
  return true;
}

void GatewayMessageHandler::remove(const TcpConnectionPtr& conn) {
  remove_by_id(conn->id());
}

void GatewayMessageHandler::remove_by_id(uint64_t conn_id) {
  client_active_times_.erase(conn_id);

  CONN_ID_TO_PEER_ID::const_iterator peer_id =
    conn_id_to_peer_id_.find(conn_id);
  // find its peer id
  if (peer_id != conn_id_to_peer_id_.end()) {
    conn_id_to_peer_id_.erase(conn_id);
    peer_id_to_conn_id_.erase(peer_id->second);
  }
  INFO_LOG("Tcp connection %ld closed\n", conn_id);
}

void GatewayMessageHandler::check_keepalive(time_t min_active_time,
                                            IDLE_CLIENTS* idle_clients) const {
  for (CLIENT_ACTIVE_TIMES::const_iterator it = client_active_times_.begin();
      it != client_active_times_.end();
      ++it) {
    printf("chek_keepalive %lu min_active_time:%lu\n",
      it->second, min_active_time);
    if (it->second < min_active_time) {
      idle_clients->push_back(it->first);
    }
  }
}

Gateway::Gateway(EventLoop* loop,
          const InetAddress& listen_addr,
          MessageHandler message_handler,
          CloseCallback conn_close_callback,
          const GatewayMessageHandler& gateway_message_handler)
  : TcpServer(loop, listen_addr, message_handler),
    conn_close_callback_(conn_close_callback),
    gateway_message_handler_(gateway_message_handler) {
}

void Gateway::connection_closed(const TcpConnectionPtr& conn) {
  conn_close_callback_(conn);
}

void Gateway::check_keepalive(int max_client_idle_time) {
	// 写日志
	common::FlushLog();

  // Remove idle clients
  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_t min_active_time = tv.tv_sec - max_client_idle_time;
  IDLE_CLIENTS idle_clients;
  gateway_message_handler_.check_keepalive(min_active_time, &idle_clients);

  for (size_t i = 0; i < idle_clients.size(); i++) {
    close_connection_by_id(idle_clients[i]);
  }

  // Shrink connection buffer
  shrink_connection_buffer();

  // Log TcpConnection's global statistics
  INFO_LOG("TcpConnectionOpened:%lu\n", TcpConnection::gTcpConnectionOpened);
  INFO_LOG("TcpConnectionClosed:%lu\n", TcpConnection::gTcpConnectionClosed);
  INFO_LOG("TcpConnectionErrorClosed:%lu\n",
         TcpConnection::gTcpConnectionErrorClosed);
  INFO_LOG("TcpConnectionData Received:%lu in Bytes\n",
        TcpConnection::gTcpConnectionDataReceived);
  INFO_LOG("TcpConnectionData Sent:%lu in Bytes\n",
        TcpConnection::gTcpConnectionDataSent);
  INFO_LOG("TcpConnectionMessage Received:%lu\n",
        TcpConnection::gTcpConnectionMessageReceived);
  INFO_LOG("TcpConnectionMessage Sent:%lu\n",
        TcpConnection::gTcpConnectionMessageSent);
}

}