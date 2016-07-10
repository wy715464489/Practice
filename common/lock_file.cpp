// Copyright [2012-2014] <HRG>
#include "common/lock_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <string.h>
#include <string>

namespace common {

void CheckIfProgramAlreadyRun(const std::string& lock_file, int* fd) {
  *fd = open(lock_file.c_str(), O_CREAT|O_RDWR, 0644);
  if (*fd < 0) {
    std::string error_msg;
    error_msg += "Failed to open lock file: ";
    error_msg += lock_file;
    perror(error_msg.c_str());
    exit(EXIT_FAILURE);
  }

  if (flock(*fd, LOCK_EX | LOCK_NB) < 0) {
    fprintf(stderr, "Error: lock_file %s was already been locked, "
            "then exit ...\n", lock_file.c_str());
    exit(EXIT_FAILURE);
  }
}

void WritePidToLockfile(int fd, pid_t pid) {
  ftruncate(fd, 0);
  char buf[100];
  snprintf(buf, sizeof(buf), "%d", pid);
  write(fd, buf, strlen(buf));
}

}  // namespace common
