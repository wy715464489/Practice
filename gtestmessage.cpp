#include <string.h>
#include <string>
#include "net/message.h"
#include "gtest/gtest.h"

using net::MessageHeader;
using net::MESSAGE_HEADER_TYPE;
using net::MESSAGE_HEADER_ID;

TEST(MessageTest, MessageHeader) {
	MessageHeader header;
  EXPECT_EQ(0, header.length());
  EXPECT_EQ(static_cast<MESSAGE_HEADER_TYPE>(0), header.type);
  EXPECT_EQ(static_cast<MESSAGE_HEADER_ID>(0), header.src_id);
  EXPECT_EQ(static_cast<MESSAGE_HEADER_ID>(0), header.dst_id);
  EXPECT_EQ(static_cast<uint32_t>(0), header.checksum);
}

using net::Message;
TEST(MessageTest, Message) {
	MessageHeader header;
	const std::string body = "hello world\n";
	Message message(header, body.c_str(), body.size());

	const uint64_t conn_id = 500;
	const uint32_t peer_ip = 127 * 256 * 256 * 256 + 1;
	message.set_owner(conn_id);
  message.set_src_ip(peer_ip);
  // Verify body
  EXPECT_EQ(body.size(), message.body_length());
  EXPECT_EQ(0, strncmp(body.c_str(), message.body(), body.size()));

  // Verify connection attribute
  EXPECT_EQ(conn_id, message.owner());
  EXPECT_EQ(peer_ip, message.src_ip());
}