// Copyright [2012-2014] <HRG>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string>
#include <sstream>
#include <map>
#include "net/eventloop.h"
#include "net/tcp_server.h"
#include "common/log.h"
#include "common/lock_file.h"
#include "testserver/test_server.h"

using net::MessageHeader;
using net::Message;
using net::MessageQueue;
using common::LogSystem;
using common::CheckIfProgramAlreadyRun;
using common::WritePidToLockfile;
using common::INFO_LOG;
using net::TcpConnection;
using testserver::TestServer;

// SIGTERM
void SigtermHandler(int sig) {
  INFO_LOG("Received SIGTERM(%d) signal, CdkeyServer will exit immediately\n",
          sig);
  TestServer::instance().exit();
}

// SIGUSR1
void Sigusr1Handler(int sig) {
  INFO_LOG("Received SIGUSR1(%d) signal, CdkeyServer will reload "
           "subsystem config files immediately\n", sig);
  TestServer::instance().reload();
}

int main(int argc, char** argv) {
  // Parse opts
  std::stringstream oss;
  std::string conf_file;
  
  std::string error_message;

  // Init log system
  LogSystem::instance().init("./log/cdkeyserver.%y%m%d%hlog", 100);
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

  // Make only one instance and write pid to lock file
  int lock_fd;
  CheckIfProgramAlreadyRun("/tmp/testserver.lock", &lock_fd);

  if (true) {
    daemon(1, 0);
  }

  // set srandom
  srandom(time(NULL));

  WritePidToLockfile(lock_fd, getpid());

  // Ignore SIGPIPE
  net::IgnorePipeSignal();

  // SIGUSR1 handler
  struct sigaction sa;
  sa.sa_handler = Sigusr1Handler;
  sa.sa_flags = 0;
  sigaction(SIGUSR1, &sa, 0);

  // SIGTERM handler
  sa.sa_handler = SigtermHandler;
  sa.sa_flags = 0;
  sigaction(SIGTERM, &sa, 0);

  net::EventLoop& loop = net::EventLoop::instance();

  testserver::TestServer::instance().init(&loop);

  loop.set_update(std::tr1::bind(&TestServer::update,
                                 &TestServer::instance()),
                  1000);  // Every one second

  // Read timezone from ENV
  tzset();
  

  loop.loop();
}
