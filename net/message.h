#ifndef NET_MESSAGE_H_
#define NET_MESSAGE_H_

#include <stdint.h>
#include <stdio.h>
#include <tr1/functional>
#include <tr1/memory>
#include <vector>
#include <string>
#include "noncopyable.h"


namespace net {

typedef uint16_t MESSAGE_HEADER_TYPE;
typedef uint32_t MESSAGE_HEADER_ID;

class TcpConnection;
typedef std::tr1::shared_ptr<TcpConnection> TcpConnectionPtr;
	// Message Header length is fixed, equals 20 Bytes
class MessageHeader {
 private:
  uint16_t  _length;    // Message total length = header_length + body_length
 public:
  MESSAGE_HEADER_TYPE type;      // Message type
  MESSAGE_HEADER_ID   src_id;    // Message sender's ID
  MESSAGE_HEADER_ID   dst_id;    // Message receiver's ID
  //uint16_t            gateway_index;
 private:
  uint32_t  _sequence;  // Sequence number, used for message sequence checking
 public:
  uint32_t  checksum;  // Used for error-checking of the header and data

  MessageHeader();
  void set_length(int len);
  int length() const;
  uint32_t sequence() const {
    return _sequence;
  }
  void set_sequence(uint32_t sequence) {
    _sequence = sequence;
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
    return _owner;
  }
  void set_owner(uint64_t owner) {
    _owner = owner;
  }

  uint32_t src_ip() const {
    return _src_ip;
  }
  void set_src_ip(uint32_t src_ip) {
    _src_ip = src_ip;
  }

  const MessageHeader& header() const {
    return *(reinterpret_cast<const MessageHeader*>(_data.c_str()));
  }

  // 设置校验和
  void set_checksum(uint32_t checksum) {
    MessageHeader* header = reinterpret_cast<MessageHeader*>(
      const_cast<char*>(_data.c_str()));
    header->checksum = checksum;
  }

  const char* body() const {
    return _data.c_str() + sizeof(MessageHeader);
  }

  size_t body_length() const {
    return _data.size() - sizeof(MessageHeader);
  }

  const char* data() const {
    return _data.c_str();
  }

  size_t data_length() const {
    return _data.size();
  }

private:
    uint64_t _owner;   // Which connection owns this message
    uint32_t _src_ip;  // The message sender's ip
    // header合并在一起
    // const MessageHeader   header_;
    std::string _data;
    DISALLOW_COPY_AND_ASSIGN(Message);
};

typedef std::vector<std::tr1::shared_ptr<Message> > MessageQueue;

// Warning: don't throw execption
// Will close the message sender's connection if return value is false
typedef std::tr1::function<bool (const Message& input,
                                 MessageQueue* outputs)> MessageHandler;

}

#endif  // NET_MESSAGE_H_