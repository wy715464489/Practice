// Copyright [2012-2014] <HRG>
#ifndef NET_MESSAGE_H_
#define NET_MESSAGE_H_

#include <stdint.h>
#include <stdio.h>
#include <tr1/functional>
#include <tr1/memory>
#include <vector>
#include <string>
#include "common/noncopyable.h"

 namespace net {

typedef uint16_t MESSAGE_HEADER_TYPE;
typedef uint32_t MESSAGE_HEADER_ID;

class TcpConnection;
typedef std::tr1::shared_ptr<TcpConnection> TcpConnectionPtr;

// Message Header length is fixed, equals 20 Bytes
class MessageHeader {
 private:
  uint16_t  length_;    // Message total length = header_length + body_length
 public:
  MESSAGE_HEADER_TYPE type;      // Message type
  MESSAGE_HEADER_ID   src_id;    // Message sender's ID
  MESSAGE_HEADER_ID   dst_id;    // Message receiver's ID
 private:
  uint32_t  sequence_;  // Sequence number, used for message sequence checking
 public:
  uint32_t  checksum;  // Used for error-checking of the header and data

  MessageHeader();
  void set_length(int len);
  int length() const;
  uint32_t sequence() const {
    return sequence_;
  }
  void set_sequence(uint32_t sequence) {
    sequence_ = sequence;
  }
};

const size_t kMessageHeaderLength = sizeof(MessageHeader);
const size_t kMaxMessageLength = 64 * 1024;  // 64 KBytes

// Switch src_id with dst_id
void ComposeGatewayMessageHeader(const MessageHeader& src_header,
                                 const std::string& body,
                                 MessageHeader* dst_header);
// only need to set length
void ComposeDBServerMessageHeader(const MessageHeader& src_header,
                                 const std::string& body,
                                 MessageHeader* dst_header);

class Message {
 public:
  Message(const MessageHeader& header,
          const char* body,
          size_t body_length);
  Message(const MessageHeader& header,
          const std::string& body);
  ~Message();

  // **Warning** owner(aka connection id) must be set
  //  otherwise TcpClient/Tcpserver does not know which connection to send
  uint64_t owner() const {
    return owner_;
  }
  void set_owner(uint64_t owner) {
    owner_ = owner;
  }

  uint32_t src_ip() const {
    return src_ip_;
  }
  void set_src_ip(uint32_t src_ip) {
    src_ip_ = src_ip;
  }

  const MessageHeader& header() const {
    return *(reinterpret_cast<const MessageHeader*>(data_.c_str()));
  }

  // 设置校验和
  void set_checksum(uint32_t checksum) {
    MessageHeader* header = reinterpret_cast<MessageHeader*>(
      const_cast<char*>(data_.c_str()));
    header->checksum = checksum;
  }

  const char* body() const {
    return data_.c_str() + sizeof(MessageHeader);
  }

  size_t body_length() const {
    return data_.size() - sizeof(MessageHeader);
  }

  const char* data() const {
    return data_.c_str();
  }

  size_t data_length() const {
    return data_.size();
  }

 private:
    uint64_t owner_;   // Which connection owns this message
    uint32_t src_ip_;  // The message sender's ip
    // header合并在一起
    // const MessageHeader   header_;
    std::string data_;
    DISALLOW_COPY_AND_ASSIGN(Message);
};

typedef std::vector<std::tr1::shared_ptr<Message> > MessageQueue;

// Warning: don't throw execption
// Will close the message sender's connection if return value is false
typedef std::tr1::function<bool (const Message& input,
                                 MessageQueue* outputs)> MessageHandler;


}  // namespace net



#endif  // NET_MESSAGE_H_
