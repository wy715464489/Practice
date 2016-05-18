#include "net/message.h"
#include <string.h>
#include <string>

namespace net {

MessageHeader::MessageHeader()
  : _length(0),
    type(0),
    src_id(0),
    dst_id(0),
    //gateway_index(0),
    _sequence(0),
    checksum(0) {
}

void MessageHeader::set_length(int len) {
  // 最大长度64K * 16 
  _length = len & 0xFFFF; 
  _sequence = (len >> 16) & 0xF;
}

int MessageHeader::length() const {
  if (_sequence == 12345) {
    // 客户端默认的所有消息,设置sequence_=12345
    return _length;
  } else {
    return ((_sequence & 0xF) << 16) + _length;
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
  : _owner(0),
    _src_ip(0) {
    // header和data合并在一起
    _data.reserve(sizeof(header) + body_length); 
    _data.append((const char*)&header, sizeof(header));
    _data.append(body, body_length);
}

Message::~Message() {
}

}