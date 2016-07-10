// Copyright [2012-2014] <HRG>
#ifndef NET_INET_ADDRESS_H_
#define NET_INET_ADDRESS_H_

#include <netinet/in.h>
#include <string>

 namespace net {

class InetAddress {
 public:
  InetAddress();

  /// Constructs an endpoint with given struct @c sockaddr_in
  /// Mostly used when accepting new connections
  explicit InetAddress(const struct sockaddr_in& addr)
    : addr_(addr) {
  }

  const struct ::sockaddr_in& addr() const { return addr_; }

  void set_addr(const struct sockaddr_in& addr) {
    addr_ = addr;
  }

  // IP string may be invalid, **MUST** check return value
  bool init(const std::string& ip, uint16_t port);

  // A string in IPv4 numbers-and-dots notation
  std::string ip() const;

  // Host byte order
  uint16_t port() const;

 private:
  struct ::sockaddr_in addr_;
};

}  // namespace net

#endif  // NET_INET_ADDRESS_H_
