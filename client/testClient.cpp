// Copyright [2012-2014] <HRG>
#include <string.h>
#include <stdio.h>
#include <string>
#include <map>
#include "net/eventloop.h"
#include "net/tcp_client.h"
#include "common/log.h"
#include "common/FSM.h"
#include "proto/communication/common_enum.pb.h"


using net::MessageHeader;
using net::Message;
using net::MessageQueue;
using net::MessageHandler;
using net::EventLoop;
using net::TcpClient;
using net::InetAddress;
using net::TcpConnectionPtr;
using common::LogSystem;

int64_t g_user_id = 0;
std::string   g_type;

bool EchoMessage(const Message& received, MessageQueue* output) {
  printf("Unknown message type:%d\n", received.header().type);
  
  return true;
}

class EchoClient: public TcpClient {
 public:
  EchoClient(EventLoop* loop,
            const InetAddress& server_addr,
            uint32_t server_id,
            MessageHandler message_handler)
  : TcpClient(loop, server_addr, message_handler) {
  }
  void connection_opened(const TcpConnectionPtr& conn) {
    MessageHeader header;

    // register
    header.set_length(sizeof(header));
    header.type = lm::REGISTER_AS_CLIENT;
    header.src_id = 0;
    header.dst_id = 0;
    conn->send(reinterpret_cast<const char*>(&header), sizeof(header));
    printf("connection opened, echo client sent the register message\n");

   
  }
};

int main(int argc, char** argv) {
  

  net::EventLoop& loop = net::EventLoop::instance();
  net::InetAddress server_addr;
  if (!server_addr.init("127.0.0.1", 8888)) {
    fprintf(stderr, "server address failed to init\n");
    exit(1);
  }
  std::tr1::shared_ptr<net::TcpClient>
    client(new EchoClient(&loop, server_addr, 101, EchoMessage));

  LogSystem::instance().init("./log/echoclient.%y%m%d%hlog", 100);
  LogSystem::instance().set_workable(common::kLogLevelDebug, true);

  client->connect();
  loop.loop();
}
