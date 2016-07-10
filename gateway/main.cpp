#include <string.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <map>
#include "net/eventloop.h"
#include "net/tcp_server.h"
#include "common/log.h"
#include "common/lock_file.h"
#include "gateway/gateway.h"

using net::MessageHeader;
using net::Message;
using net::MessageQueue;
using common::LogSystem;
using common::ERROR_LOG;
using common::DEBUG_LOG;

using common::CheckIfProgramAlreadyRun;
using common::WritePidToLockfile;
using gateway::Gateway;
using net::TcpConnection;
using gateway::GatewayMessageHandler;
GatewayMessageHandler message_handler;

int main(int argc, char** argv) {
	// Init log system
  LogSystem::instance().init("./log/gateway.%y%m%d%hlog", 100);
 	if (!LogSystem::instance().set_workable("debug", true)) {
      fprintf(stderr, "Unknown workable log level: %s\n",
             "debug");
  }
  if (!LogSystem::instance().set_workable("info", true)) {
      fprintf(stderr, "Unknown workable log level: %s\n",
             "info");
  }
  if (!LogSystem::instance().set_workable("error", true)) {
      fprintf(stderr, "Unknown workable log level: %s\n",
             "error");
  }
  if (!LogSystem::instance().set_workable("data", true)) {
      fprintf(stderr, "Unknown workable log level: %s\n",
             "data");
  }

  fprintf(stderr, "Unknown workable log level: %s\n",
             "debug");
  
  // Make only one instance and write pid to lock file
  int lock_fd;
  CheckIfProgramAlreadyRun("/tmp/gateway.lock", &lock_fd);

  if (true) {
    daemon(1, 0);
  }
  common::FlushLog();
  DEBUG_LOG("sfsdf");
  // Ignore SIGPIPE
  net::IgnorePipeSignal();

  WritePidToLockfile(lock_fd, getpid());

  net::EventLoop& loop = net::EventLoop::instance();
  net::InetAddress listen_addr;
  if (!listen_addr.init("0.0.0.0", 8888)) {
    ERROR_LOG("Failed to init listen addr, port:%d\n", 8888);
    exit(EXIT_FAILURE);
  }

  gateway::Gateway gateway(&loop,
                            listen_addr,
                            std::tr1::bind(&GatewayMessageHandler::handle_message,
                                           &message_handler,
                                           std::tr1::placeholders::_1,
                                           std::tr1::placeholders::_2),
                            std::tr1::bind(&GatewayMessageHandler::remove,
                                           &message_handler,
                                           std::tr1::placeholders::_1),
                            message_handler
                            );
  gateway.start();

  loop.set_update(std::tr1::bind(&Gateway::check_keepalive,
                                 &gateway,
                                 5000),
                  5000);
  loop.loop();
  printf("Hello, I am gateway server, ^_^\n");
}