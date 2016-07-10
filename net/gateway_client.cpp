#include "net/gateway_client.h"
#include "proto/communication/common_enum.pb.h"

namespace net {

GatewayClient::GatewayClient(net::EventLoop* loop,
                             const net::InetAddress& gateway_addr,
                             MessageHandler message_handler,
                             int global_id)
  : TcpClient(loop, gateway_addr, message_handler),
    global_id_(global_id) {
}

GatewayClient::~GatewayClient() {
}

void GatewayClient::connection_opened(const TcpConnectionPtr& conn) {
  net::MessageHeader header;

  // register
  header.set_length(sizeof(header));
  header.type = lm::REGISTER_AS_SERVER;
  header.src_id = global_id_;
  header.dst_id = 0;
  // check send's return value if want to send again
  conn->send(reinterpret_cast<const char*>(&header), sizeof(header));
  printf("connection opened, sent gateway register message\n");
}

}  // namespace net
