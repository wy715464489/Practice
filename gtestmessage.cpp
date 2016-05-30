#include <string.h>
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
#include "gtest/gtest.h"

// using net::MessageHeader;
// using net::MESSAGE_HEADER_TYPE;
// using net::MESSAGE_HEADER_ID;

// using net::TcpConnection;
// using net::TcpConnectionPtr;
// using net::Poller;
// using net::Buffer;
// using net::IgnorePipeSignal;
// using net::TcpClient;
// using net::TcpServer;
// using net::Message;
// using net::MessageQueue;
// using net::MessageHeader;
// using net::MessageHandler;

int gFuncExecNum = 0;

// class TcpClientNormal: public TcpClient {
//  public:
//    TcpClientNormal(net::EventLoop* loop,
//                   const net::InetAddress& server_addr,
//                   MessageHandler message_handler)
//   : TcpClient(loop, server_addr, message_handler) {
//   }

//  private:
//    virtual void connection_opened(const TcpConnectionPtr& conn) {
//     ++gFuncExecNum;
//     std::string s = "hello world";
//     MessageHeader header;
//     header.set_length(sizeof(header)  + s.size());
//     header.type = 30;
//     header.src_id = 7;
//     header.dst_id = 10;
    
//     header.checksum = 0;
//     EXPECT_TRUE(conn->send(reinterpret_cast<const char*>(&header),
//                 sizeof(header)));
//     EXPECT_TRUE(conn->send(s.c_str(), s.size()));
//     printf("client sent body:%s\n", s.c_str());
//   }
// };

// bool ClientHandleNormal(const Message& input, MessageQueue* outputs) {
  // ++gFuncExecNum;
  // std::string body(input.body(), input.body_length());
  // printf("client received body:%s\n", body.c_str());
  // // EXPECT_EQ(std::string("hello world"), body);
  // net::EventLoop::instance().quit();
  // return true;
// }

// bool ServerHandleNormal(const Message& input, MessageQueue* outputs) {
  // ++gFuncExecNum;
  // printf("server received message, %u\n", input.header().length());
 // }

TEST(MessageTest, MessageHeader) {
	// MessageHeader header;
 //  EXPECT_EQ(0, header.length());
 //  EXPECT_EQ(static_cast<MESSAGE_HEADER_TYPE>(0), header.type);
 //  EXPECT_EQ(static_cast<MESSAGE_HEADER_ID>(0), header.src_id);
 //  EXPECT_EQ(static_cast<MESSAGE_HEADER_ID>(0), header.dst_id);
 //  EXPECT_EQ(static_cast<uint32_t>(0), header.checksum);
}

// using net::Message;
TEST(MessageTest, Message) {
	// MessageHeader header;
	// const std::string body = "hello world\n";
	// Message message(header, body.c_str(), body.size());

	// const uint64_t conn_id = 500;
	// const uint32_t peer_ip = 127 * 256 * 256 * 256 + 1;
	// message.set_owner(conn_id);
 //  message.set_src_ip(peer_ip);
 //  // Verify body
 //  EXPECT_EQ(body.size(), message.body_length());
 //  EXPECT_EQ(0, strncmp(body.c_str(), message.body(), body.size()));

 //  // Verify connection attribute
 //  EXPECT_EQ(conn_id, message.owner());
 //  EXPECT_EQ(peer_ip, message.src_ip());
}


// TEST(TcpClientServerTest, NormalRecvAndSend) {
 // net::EventLoop& loop = net::EventLoop::instance();
 // net::InetAddress server_addr;
 // server_addr.init("127.0.0.1", 9999);
 // {
 //    TcpServer server(&loop, server_addr, ServerHandleNormal);
 //    server.start();

 //    TcpClientNormal client(&loop, server_addr, ClientHandleNormal);
 //    client.connect();

 //    loop.loop();

 //    EXPECT_EQ(0, gFuncExecNum);
 //    gFuncExecNum = 0;
 //  }
// }