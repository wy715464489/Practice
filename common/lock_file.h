// Copyright [2012-2014] <HRG>
#ifndef COMMON_LOCK_FILE_H_
#define COMMON_LOCK_FILE_H_

#include <sys/types.h>
#include <unistd.h>
#include <string>

namespace common {

void CheckIfProgramAlreadyRun(const std::string& lock_file, int* fd);

void WritePidToLockfile(int fd, pid_t pid);

}  // namespace common

#endif  // COMMON_LOCK_FILE_H_
