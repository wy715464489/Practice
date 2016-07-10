// Copyright [2012-2014] <HRG>
#ifndef COMMON_LOG_H_
#define COMMON_LOG_H_

#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <vector>
#include <string>
#include "common/noncopyable.h"

namespace common {

typedef std::vector<std::string> RULE_LIST;

const int kDefaultRollingPeriod = 24 * 60;  // One day

bool SplitLogname(const std::string& logname,
                  RULE_LIST* rules,
                  int* rolling_period);

void ComposeLogfilename(const RULE_LIST& rules,
                        const std::string& level,
                        const timeval& tv,
												const time_t rolling_time,
                        std::string* filename);

// filename a.txt ./a.txt /a.txt /m/n/a.txt
bool CreateFiledirectory(const std::string& filename);

class LogFileObject {
 public:
  LogFileObject();
  ~LogFileObject();
  // logname:       e.g. /tmp/log/%y-%m-%d/gamesrv-%h-%i.log
  //                %y - years since 1900
  //                %m - month [01, 12]
  //                %d - month day [01, 31]
  //                %h - hour [00, 23]
  //                %i - minutes [00, 59]
  // level:         debug,info,error,data
  // max_log_size:  in MB
  // rolling time:  default: every day, %h: every hour %i: every minute
  //                also supports every n minutes, e.g. %5i: every 5 mitutes
  // logname postfix: append time(in us) and level to make logname different
  //                  e.g.: gamesrvlog.1402450278232370.info
  bool init(const std::string& logname,
            const std::string& level,
            int max_log_size);

  void set_workable(bool workable);

  void write(const char* content, ...);
  void write(const char* content, va_list vl);

  void check_filesize();
  void check_rolling_time();

  // For unittest
  const std::string& filename() const {
    return filename_;
  }
  // For unittest
  void set_rolling_period(int rolling_period) {
    rolling_period_ = rolling_period;
  }

  // 把所有日志刷到磁盘
  void flush() {
    if (file_ != NULL) {
      fflush(file_);
    }
  }

 private:

  bool workable_;
  bool has_inited_;  // Init only once
  RULE_LIST rules_;
  std::string level_;
  int max_log_size_;
  FILE*   file_;
  std::string filename_;
  int rolling_period_;
  time_t next_rolling_time_;
  DISALLOW_COPY_AND_ASSIGN(LogFileObject);
};


const std::string kLogLevelDebug = "debug";
const std::string kLogLevelInfo = "info";
const std::string kLogLevelError = "error";
const std::string kLogLevelData = "data";

class LogSystem {
 public:
  static LogSystem& instance() {
    static LogSystem log_system;
    return log_system;
  }

  // **Warning** Must init log system before use
  bool init(const std::string& logname, int max_log_size);
  bool init_data_log(const std::string& logname, int max_log_size);

  bool set_workable(const std::string& level, bool workable);

  LogFileObject& debug() {
    return debug_;
  }
  LogFileObject& info() {
    return info_;
  }
  LogFileObject& error() {
    return error_;
  }
  LogFileObject& data() {
    return data_;
  }

 private:
  LogSystem() {
  }

  LogFileObject debug_;
  LogFileObject info_;
  LogFileObject error_;
  LogFileObject data_;
  DISALLOW_COPY_AND_ASSIGN(LogSystem);
};

extern void DEBUG_LOG(const char* content, ...) __attribute__((format(printf,1,2)));
extern void INFO_LOG(const char* content, ...) __attribute__((format(printf,1,2)));
extern void ERROR_LOG(const char* content, ...) __attribute__((format(printf,1,2)));
extern void DATA_LOG(const char* content, ...) __attribute__((format(printf,1,2)));

//#define DEBUG_LOG LogSystem::instance().debug().write
//#define INFO_LOG  LogSystem::instance().info().write
//#define ERROR_LOG LogSystem::instance().error().write
//#define DATA_LOG LogSystem::instance().data().write

void FlushLog();

void SigsegvHandler(int sig);

#define DATA_INFO_LOG(protobuf_obj) \
  DATA_LOG("\t%s\t%s\n", protobuf_obj.GetDescriptor()->name().c_str(), \
                         protobuf_obj.ShortDebugString().c_str())

}  // namespace common



#endif  // COMMON_LOG_H_
