#include "inet_address.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

InetAddress::InetAddress() {
	bzero(&_addr, sizeof(_addr));
}

bool InetAddress::init(const std::string& ip, uint16_t port) {
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(port);
	if (::inet_pton(AF_INET, ip.c_str(), &_addr.sin_addr) <= 0) {
		// ERROR_LOG("InetAddress::init failed, ip: %s, port: %u, error:%s\n",
		// 		ip.c_str(), port, strerror(errno));
	return false;
  }
  return true;
}

std::string InetAddress::ip() const {
	return inet_ntoa(_addr.sin_addr);
}

uint16_t InetAddress::port() const {
  return ntohs(_addr.sin_port);
}