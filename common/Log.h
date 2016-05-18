#ifndef COMMON_LOG_H_
#define COMMON_LOG_H_

#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <vector>
#include <string>


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

class LogFileObject
{
 public:
  LogFileObject();
  ~LogFileObject();

  bool init(const std::string& logname,
        const std::string& level,
        int max_log_size);

  void set_workable(bool workable);
  void write(const char* content, ...);
  void write(const char* content, va_list vl);

  void check_filesize();
  void check_rolling_time();

  void flush() {
    if (_file != NULL)
    {
      fflush(_file);
    }
  }

 protected:

  bool _workable;
  bool _has_inited;  // Init only once
  RULE_LIST _rules;
  std::string _level;
  int _max_log_size;
  FILE*   _file;
  std::string _filename;
  int _rolling_period;
  time_t _next_rolling_time;
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

  bool init(const std::string& logname, int max_log_size);
  bool init_data_log(const std::string& logname, int max_log_size);

  bool set_workable(const std::string& level, bool workable);

  LogFileObject& debug() {
    return _debug;
  }
  LogFileObject& info() {
    return _info;
  }
  LogFileObject& error() {
    return _error;
  }
  LogFileObject& data() {
    return _data;
  }
 private:
  LogSystem() {
  }

  LogFileObject _debug;
  LogFileObject _info;
  LogFileObject _error;
  LogFileObject _data;
  DISALLOW_COPY_AND_ASSIGN(LogSystem);
};

extern void debug_log(const char* content, ...) __attribute__((format(printf,1,2)));
extern void info_log(const char* content, ...) __attribute__((format(printf,1,2)));
extern void error_log(const char* content, ...) __attribute__((format(printf,1,2)));
extern void DATA_LOG(const char* content, ...) __attribute__((format(printf,1,2)));

#define ERROR_LOG(content, ...) \
    error_log("(%s:%d)[ERROR] "content, __FILE__, __LINE__, ##__VA_ARGS__);

#define DEBUG_LOG(content, ...) \
    debug_log("(%s:%d)[DEBUG] "content, __FILE__, __LINE__, ##__VA_ARGS__);

#define INFO_LOG(content, ...) \
    info_log("(%s:%d)[INFO] "content, __FILE__, __LINE__, ##__VA_ARGS__);

void FlushLog();

void SigsegvHandler(int sig);

#define DATA_INFO_LOG(protobuf_obj) \
  DATA_LOG("\t%s\t%s\n", protobuf_obj.GetDescriptor()->name().c_str(), \
                         protobuf_obj.ShortDebugString().c_str())
}

#endif  // COMMON_LOG_H_
