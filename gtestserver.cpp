// Copyright [2012-2014] <HRG>
#include <tr1/functional>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string>
#include "net/eventloop.h"
#include "net/inet_address.h"
#include "net/tcp_connection.h"
#include "net/socket_ops.h"
#include "net/channel.h"
#include "net/poller.h"
#include "net/buffer.h"
#include "net/tcp_client.h"
#include "net/tcp_server.h"
#include "common/log.h"
#include "gtest/gtest.h"

using hrg::net::TcpConnection;
using hrg::net::TcpConnectionPtr;
using hrg::net::Poller;
using hrg::net::Buffer;
using hrg::net::IgnorePipeSignal;
using hrg::net::TcpClient;
using hrg::net::TcpServer;
using hrg::net::Message;
using hrg::net::MessageQueue;
using hrg::net::MessageHeader;
using hrg::net::MessageHandler;
using hrg::common::LogSystem;

int gFuncExecNum1 = 0;

class TcpClientNormal: public TcpClient {
 public:
  TcpClientNormal(hrg::net::EventLoop* loop,
                  const hrg::net::InetAddress& server_addr,
                  MessageHandler message_handler)
  : TcpClient(loop, server_addr, message_handler) {
  }

 private:
  virtual void connection_opened(const TcpConnectionPtr& conn) {
    ++gFuncExecNum1;
    std::string s = "hello world";
    MessageHeader header;
    header.set_length(sizeof(header) + s.size());
    header.type = 30;
    header.src_id = 7;
    header.dst_id = 10;
    
    header.checksum = 0;
    EXPECT_TRUE(conn->send(reinterpret_cast<const char*>(&header),
                sizeof(header)));
    EXPECT_TRUE(conn->send(s.c_str(), s.size()));
    printf("client sent body:%s\n", s.c_str());
  }
};

class TcpClientNormalSend: public TcpClient {
 public:
  TcpClientNormalSend(hrg::net::EventLoop* loop,
                  const hrg::net::InetAddress& server_addr,
                  MessageHandler message_handler)
  : TcpClient(loop, server_addr, message_handler) {
  }

 private:
  virtual void connection_opened(const TcpConnectionPtr& conn) {
    ++gFuncExecNum1;
    std::string s = "hello world";
    MessageHeader header;
    header.set_length( sizeof(header) + s.size());
    header.type = 30;
    header.src_id = 7;
    header.dst_id = 10;
    
    header.checksum = 0;

    EXPECT_TRUE(send(reinterpret_cast<const char*>(&header), sizeof(header)));
    EXPECT_TRUE(send(s.c_str(), s.size()));
    // printf("client sent body:%s\n", s.c_str());
  }
};

bool ClientHandleNormal1(const Message& input, MessageQueue* outputs) {
  ++gFuncExecNum1;
  std::string body(input.body(), input.body_length());
  // printf("client received body:%s\n", body.c_str());
  EXPECT_EQ(std::string("hello world"), body);
  hrg::net::EventLoop::instance().quit();
  return true;
}

bool ServerHandleNormal1(const Message& input, MessageQueue* outputs) {
  ++gFuncExecNum1;
  printf("server received message, %u\n", input.header().length());
  MessageHeader header;
  header.set_length(input.header().length());
  header.type = input.header().type;
  header.src_id = input.header().dst_id;
  header.dst_id = input.header().src_id;
  
  header.checksum = 0;

  std::tr1::shared_ptr<Message> out_message(new Message(
                           header,
                           input.body(),
                           input.body_length()));
  out_message->set_owner(input.owner());
  outputs->push_back(out_message);
  return true;
}

TEST(TcpClientServerTest, NormalRecvAndSend) {
  hrg::net::EventLoop& loop = hrg::net::EventLoop::instance();
  hrg::net::InetAddress server_addr;
  server_addr.init("127.0.0.1", 9999);

  /*
  LogSystem::instance().init("./log/client_server_test.%y%m%d%hlog", 100);
  LogSystem::instance().set_workable(hrg::common::kLogLevelDebug, true);
  LogSystem::instance().set_workable(hrg::common::kLogLevelInfo, true);
  LogSystem::instance().set_workable(hrg::common::kLogLevelError, true);
  */

  // use conn->send
  {
    TcpServer server(&loop, server_addr, ServerHandleNormal1);
    server.start();

    TcpClientNormal client(&loop, server_addr, ClientHandleNormal1);
    client.connect();

    loop.loop();

    EXPECT_EQ(3, gFuncExecNum1);
    gFuncExecNum1 = 0;
  }

  // use client->send
  // {
  //   TcpServer server(&loop, server_addr, ServerHandleNormal1);
  //   server.start();

  //     TcpClientNormalSend client(&loop, server_addr, ClientHandleNormal1);
  //   client.connect();

  //   loop.loop();

  //   EXPECT_EQ(3, gFuncExecNum1);
  //   gFuncExecNum1 = 0;
  // }
}

// TEST(TcpClientServerTest, ClientStartFirst) {
//   hrg::net::EventLoop& loop = hrg::net::EventLoop::instance();
//   hrg::net::InetAddress server_addr;
//   server_addr.init("127.0.0.1", 9999);

//   TcpClientNormal client(&loop, server_addr, ClientHandleNormal);
//   client.connect();

//   sleep(2);

//   TcpServer server(&loop, server_addr, ServerHandleNormal);
//   server.start();

//   loop.loop();

//   EXPECT_EQ(3, gFuncExecNum1);
//   gFuncExecNum1 = 0;
// }

// class TcpClientCloseServerBeforeSend: public TcpClient {
//  public:
//   TcpClientCloseServerBeforeSend(hrg::net::EventLoop* loop,
//                   const hrg::net::InetAddress& server_addr,
//                   MessageHandler message_handler,
//                   TcpServer* server)
//   : TcpClient(loop, server_addr, message_handler),
//     server_(server) {
//   }

//  private:
//   virtual void connection_opened(const TcpConnectionPtr& conn) {
//     ++gFuncExecNum1;

//     // close server
//     delete server_;

//     std::string s = "hello world";
//     MessageHeader header;
//     header.set_length(sizeof(header) + s.size());
//     header.type = 30;
//     header.src_id = 0;
//     header.dst_id = 10;
    
//     header.checksum = 0;
//     EXPECT_TRUE(conn->send(reinterpret_cast<const char*>(&header),
//                            sizeof(header)));
//     EXPECT_FALSE(conn->send(s.c_str(), s.size()));

//     hrg::net::EventLoop::instance().quit();
//   }
//   TcpServer* server_;
// };

// TEST(TcpClientServerTest, CloseServerBeforeClientSend) {
//   // Set signal handler
//   // Pipe signal will be raised when try to write a closed socket
//   IgnorePipeSignal();

//   hrg::net::EventLoop& loop = hrg::net::EventLoop::instance();
//   hrg::net::InetAddress server_addr;
//   server_addr.init("127.0.0.1", 9999);

//   TcpServer* server = new TcpServer(&loop, server_addr, ServerHandleNormal);
//   server->start();

//   TcpClientCloseServerBeforeSend
//     client(&loop, server_addr, ClientHandleNormal, server);
//   client.connect();

//   loop.loop();

//   EXPECT_EQ(1, gFuncExecNum1);
//   gFuncExecNum1 = 0;
// }

// void ExitEventLoop() {
//   hrg::net::EventLoop::instance().quit();
// }

// bool ServerHandleCloseClientBeforeSend(TcpClient* client,
//                                        const Message& input,
//                                        MessageQueue* outputs) {
//   ++gFuncExecNum1;
//   delete client;

//   MessageHeader header;
//   header.set_length(input.header().length());
//   header.type = input.header().type;
//   header.src_id = input.header().dst_id;
//   header.dst_id = input.header().src_id;
  
//   header.checksum = 0;

//   std::tr1::shared_ptr<Message> out_message(new Message(
//                            header,
//                            input.body(),
//                            input.body_length()));
//   out_message->set_owner(input.owner());
//   // send two messages
//   outputs->push_back(out_message);
//   outputs->push_back(out_message);
//   hrg::net::EventLoop::instance().run_after(100, ExitEventLoop);
//   return true;
// }


// TEST(TcpClientServerTest, CloseClientBeforeServerSend) {
//   // Set signal handler
//   // Pipe signal will be raised when try to write a closed socket
//   IgnorePipeSignal();

//   hrg::net::EventLoop& loop = hrg::net::EventLoop::instance();
//   hrg::net::InetAddress server_addr;
//   server_addr.init("127.0.0.1", 9999);

//   TcpClientNormal* client =
//     new TcpClientNormal(&loop, server_addr, ClientHandleNormal);
//   client->connect();

//   TcpServer* server = new TcpServer(&loop, server_addr,
//                       std::tr1::bind(ServerHandleCloseClientBeforeSend, client,
//                                   std::tr1::placeholders::_1,
//                                   std::tr1::placeholders::_2));
//   server->start();

//   loop.loop();

//   EXPECT_EQ(2, gFuncExecNum1);
//   gFuncExecNum1 = 0;
// }
