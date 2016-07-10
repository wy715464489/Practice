// Copyright [2012-2014] <HRG>
#include "net/inet_address.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include "common/log.h"
using common::LogSystem;
using common::ERROR_LOG;

 namespace net {

InetAddress::InetAddress() {
  bzero(&addr_, sizeof addr_);
}

bool InetAddress::init(const std::string& ip, uint16_t port) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);
  if (::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0) {
    ERROR_LOG("InetAddress::init failed, ip: %s, port: %u, error:%s\n",
             ip.c_str(), port, strerror(errno));
    return false;
  }
  return true;
}

std::string InetAddress::ip() const {
  return inet_ntoa(addr_.sin_addr);
}

uint16_t InetAddress::port() const {
  return ntohs(addr_.sin_port);
}

}  // namespace net

