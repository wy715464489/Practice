// Copyright [2012-2014] <HRG>
#ifndef NET_SOCKET_OPS_H_
#define NET_SOCKET_OPS_H_

#include <netinet/in.h>
#include <errno.h>
#include <string>

namespace hrg { namespace net {

// Ignore Pipe signal when write to a closed/reset socket
void IgnorePipeSignal();

// Create a socket, die if failed
int CreateNonblockingSocketOrDie();

// Bind the socket, die if failed
void BindOrDie(int fd, const struct sockaddr_in& addr);

// Listen, die if failed
void ListenOrDie(int fd);

enum ConnectStatus {
  kConnectContinue,  // Conntinue next step
  kConnectRetry,  // Retry next time
  kConnectFatalError,  // Fatal error occurs
};

ConnectStatus Connect(int fd, const struct sockaddr_in& addr);

int GetSocketError(int fd);

// Converts cp, a string in IPv4 numbers-and-dots notation,
// into a number in host byte order
// On success, the converted address is returned.
// If the input is invalid, -1 is returned.
uint32_t AddressToNetwork(const std::string& ip);

// converts the Internet host address in, given in host byte order,
// to a string in IPv4 dotted-decimal notation
const char* NetworkToAddress(uint32_t ip);

// A类：10.0.0.0 ~ 10.255.255.255
// B类：172.16.0.0 ~ 172.31.255.255
// C类：192.168.0.0 ~ 192.168.255.255
// localhost: 127.0.0.1
// ip is in host byte order
bool IsInternalIp(uint32_t ip);

}  // namespace net
}  // namespace hrg
#endif  // NET_SOCKET_OPS_H_
