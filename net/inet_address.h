#ifndef NET_INET_ADDRESS_H_
#define NET_INET_ADDRESS_H_

#include <netinet/in.h>
#include <string>

namespace net{

class InetAddress
{
 public:
	InetAddress();
	
	InetAddress(const struct sockaddr_in& addr)
		: _addr(addr) {
	}

	const struct ::sockaddr_in& addr() const { 
		return _addr; 
	}

	void set_addr(const struct sockaddr_in& addr) {
		_addr = addr;
	}

	bool init(const std::string& ip, uint16_t port);

	std::string ip() const;

	uint16_t port() const;	

 private:
 	struct ::sockaddr_in _addr;
};

}
#endif  // NET_INET_ADDRESS_H_