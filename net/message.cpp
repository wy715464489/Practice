// Copyright [2012-2014] <HRG>
#include "net/message.h"
#include <string.h>
#include <string>

namespace hrg { namespace net {

MessageHeader::MessageHeader()
  : length_(0),
    type(0),
    src_id(0),
    dst_id(0),
    sequence_(0),
    checksum(0) {
}

void MessageHeader::set_length(int len) {
  // 最大长度64K * 16 
  length_ = len & 0xFFFF; 
  sequence_ = (len >> 16) & 0xF;
}

int MessageHeader::length() const {
  if (sequence_ == 12345) {
    // 客户端默认的所有消息,设置sequence_=12345
    return length_;
  } else {
    return ((sequence_ & 0xF) << 16) + length_;
  }
}

void ComposeGatewayMessageHeader(const MessageHeader& src_header,
                                 const std::string& body,
                                 MessageHeader* dst_header) {
  dst_header->set_length(sizeof(MessageHeader) + body.size());
  dst_header->type = src_header.type;
  dst_header->src_id = src_header.dst_id;
  dst_header->dst_id = src_header.src_id;
}

Message::Message(const MessageHeader& header,
                 const char* body,
                 size_t body_length)
  : owner_(0),
    src_ip_(0) {
    // header和data合并在一起
    data_.reserve(sizeof(header) + body_length); 
    data_.append((const char*)&header, sizeof(header));
    data_.append(body, body_length);
}

Message::Message(const MessageHeader& header,
                 const std::string& body)
  : owner_(0),
    src_ip_(0) {
  // header和data合并在一起
  data_.reserve(sizeof(header) + body.size()); 
  data_.append((const char*)&header, sizeof(header));
  data_.append(body);
}

Message::~Message() {
}

}  // namespace net
}  // namespace hrg
