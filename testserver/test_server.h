#ifndef TEST_SERVER_H_
#define TEST_SERVER_H_

#include <map>
#include <vector>
#include <string>
#include "net/message.h"
#include "net/eventloop.h"
#include "net/gateway_client.h"
#include "common/FSM.h"

namespace testserver {

using net::Message;
using net::MessageQueue;
using net::GatewayClient;

typedef std::tr1::function<void (int client_idx,
                           const Message& message)> TestServerMessageHandler;

typedef std::map<int, TestServerMessageHandler> TestServerMessageHandlers;


class TestServer {
 public:
  static TestServer& instance() {
    static TestServer test_server;
    return test_server;
  }

  // Exit directly if init failed
  void init(net::EventLoop* loop);

  int id() const;

  bool handle_message(int client_idx,  // may connect several gateways
                      const Message& message);

  // Exit directly if this type has already been registered
  void register_message_handler(int type,
                                TestServerMessageHandler message_handler);

  void send_message_through_gateway(const Message& message);
  void send_message_through_gateway(int gateway_id, const Message& message);

  void UseCDkeyRequest(int gateway_id, const Message& message);

  void connect_gateway();

  void exit();

  void reload();

  void update();

  void subsystem_reload();

  void subsystem_register();

 private:
  TestServer();
  ~TestServer();

  // FSM
  void      onIdentityEnter();
  void      onIdentityExit();
  void      onIdentityTick(float fdt);
  void      onNewsEnter();
  void      onNewsExit();
  void      onNewsTick(float fdt);
  
  std::string subsystem_conf_;
  int       sample_message_;
  int       id_;
  bool			exit_;
  bool      reload_;
  int64_t   keep_alive_;

  std::vector<GatewayClient*>        gateway_clients_;
  TestServerMessageHandlers          message_handlers_;
  net::EventLoop*               loop_;

  FSM     mFSM; 
};

}
#endif