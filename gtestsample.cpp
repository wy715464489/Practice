#include "gtest/gtest.h"
#include "net/inet_address.h"
#include <string>

TEST(InetAddressTest, IpAndPort)
{
  std::string ips[] = {
    "0.0.0.0",
    "10.10.2.1",
    "127.0.0.0",
    "192.168.1.1",
    "225.1.33.11",
    "255.255.255.255"
  };

  uint16_t ports[] = {
    0,
    10,
    200,
    500,
    6000,
    35661
  };
  EXPECT_EQ(sizeof(ips)/sizeof(ips[0]),
          sizeof(ports)/sizeof(ports[0]));

  for (size_t i = 0; i < sizeof(ips)/sizeof(ips[0]); i++) {
    InetAddress addr;
    EXPECT_TRUE(addr.init(ips[i], ports[i]));
    EXPECT_EQ(ips[i], addr.ip());
    EXPECT_EQ(ports[i], addr.port());
  }

  std::string invalid_ips[] = {"1234.4.2.1",
                                "100.123.0",
                                "a.b.c"
                              };
  for (size_t i = 0; i < sizeof(invalid_ips)/sizeof(invalid_ips[0]); i++) {
    InetAddress addr;
    EXPECT_FALSE(addr.init(invalid_ips[i],100));
  }
}