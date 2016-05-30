// Copyright [2012-2014] <HRG>
#include "common/log.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string>

namespace hrg { namespace common {

bool SplitLogname(const std::string& logname,
                  RULE_LIST* rules,
                  int* rolling_period) {
  size_t curr_pos = 0;
  size_t next_pos = 0;
  *rolling_period = kDefaultRollingPeriod;  // Default one day

  if (logname.empty()) {
    return false;
  }

  while ((next_pos = logname.find_first_of('%', curr_pos))
          != std::string::npos) {
    if (next_pos != curr_pos) {
      rules->push_back(logname.substr(curr_pos, next_pos - curr_pos));
    }
    // Must has date char behind %
    if ((next_pos + 1) == logname.size()) {
      return false;
    }
    int period = *rolling_period;
    char behind = logname[next_pos + 1];
    if (behind == 'y' || behind == 'm' || behind == 'd'||
        behind == 'h' || behind == 'i') {
      if (behind == 'h') {
        period = 60;
      } else if (behind == 'i') {
        period = 1;
      }
      rules->push_back(logname.substr(next_pos, 2));
      curr_pos = next_pos + 2;
    } else if (behind >= '0' && behind <= '9') {
      // e.g. %5i, %10i, %15i
      size_t minutes_pos = logname.find_first_of('i', next_pos + 1);
      if (minutes_pos == std::string::npos) {
        return false;
      }
      period = atoi(logname.c_str() + next_pos + 1);
      if (period <= 0) {
        period = 1;
      }
      rules->push_back("%i");
      curr_pos = minutes_pos + 1;
    } else {
      return false;
    }

    if (period < *rolling_period) {
      *rolling_period = period;
    }
  }

  if (curr_pos  < logname.size()) {
    rules->push_back(logname.substr(curr_pos));
  }
  return true;
}

void ComposeLogfilename(const RULE_LIST& rules,
                        const std::string& level,
                        const timeval& tv,
												time_t rolling_time,
                        std::string* filename) {
  filename->clear();
  struct tm rollingime_tm;
  localtime_r(&rolling_time, &rollingime_tm);

  char buffer[100] = {0};

  for (RULE_LIST::const_iterator it = rules.begin();
      it != rules.end(); ++it) {
    const std::string& rule = *it;
    if (rule.size() == 2 && rule[0] == '%') {
      if (rule == "%y") {
        // Years since 1900
        snprintf(buffer, sizeof(buffer), "%04d", rollingime_tm.tm_year + 1900);
      } else if (rule == "%m") {
        // Range [01, 12]
        snprintf(buffer, sizeof(buffer), "%02d", rollingime_tm.tm_mon + 1);
      } else if (rule == "%d") {
        snprintf(buffer, sizeof(buffer), "%02d", rollingime_tm.tm_mday);
      } else if (rule == "%h") {
        snprintf(buffer, sizeof(buffer), "%02d", rollingime_tm.tm_hour);
      } else if (rule == "%i") {
        snprintf(buffer, sizeof(buffer), "%02d", rollingime_tm.tm_min);
      } else {
        // Should never happen, just in case
        fprintf(stderr, "Unknown logfilename rule: %s\n", rule.c_str());
      }
      *filename += buffer;
    } else {
      *filename += rule;
    }
  }

  // In order to make filename different, append time(in microseconds) and level
  // e.g.: gamesrvlog.1402450278232370.info
  snprintf(buffer, sizeof(buffer), ".%ld", tv.tv_sec * 1000000 + tv.tv_usec);
  *filename += buffer;
  *filename += ".";
  *filename += level;
}

// filename a.txt ./a.txt /a.txt /m/n/a.txt
bool CreateFiledirectory(const std::string& filename) {
  size_t pos = 0;
  while ((pos = filename.find_first_of('/', pos)) != std::string::npos) {
    pos += 1;
    const std::string filedir = filename.substr(0, pos);
    if (mkdir(filedir.c_str(), 0777) == -1) {
      if (errno != EEXIST) {
        std::string errno_msg = "Failed to make directory: ";
        errno_msg += filedir;
        perror(errno_msg.c_str());
        return false;
      }
    }
  }
  return true;
}

LogFileObject::LogFileObject()
  : workable_(false),
    has_inited_(false),
    level_("info"),
    max_log_size_(1000),  // 1GB
    file_(NULL),
    rolling_period_(kDefaultRollingPeriod) {
}

bool LogFileObject::init(const std::string& logname,
                         const std::string& level,
                         int max_log_size) {
  // At most init once
  if (has_inited_) {
    return true;
  }

  level_ = level;
  max_log_size_ = max_log_size;
  rules_.clear();

  if (!SplitLogname(logname, &rules_, &rolling_period_)) {
    fprintf(stderr, "Wrong logname format: %s\n", logname.c_str());
    return false;
  }

  // Minutes to seconds
  rolling_period_ *= 60;
  has_inited_ = true;
  return true;
}

LogFileObject::~LogFileObject() {
  if (file_ != NULL) {
    fclose(file_);
    file_ = NULL;
  }
}

void LogFileObject::write(const char* content, ...) {
    va_list vl;
    va_start(vl, content);
    write(content, vl);
    va_end(vl);
}

void LogFileObject::write(const char* content, va_list vl) {
  if (!workable_ || !has_inited_) {
    return;
  }

  check_rolling_time();
  check_filesize();

  if (file_ == NULL) {
    timeval tv;
    gettimeofday(&tv, NULL);
    time_t now_rolling_time = tv.tv_sec / rolling_period_ * rolling_period_;
    ComposeLogfilename(rules_, level_, tv, now_rolling_time, &filename_);
    if (!CreateFiledirectory(filename_)) {
      fprintf(stderr, "Failed to create directory: %s\n", filename_.c_str());
      return;
    }

    file_ = fopen(filename_.c_str(), "a");
    if (file_ == NULL) {
      fprintf(stderr, "Failed to open file to append: %s\n", filename_.c_str());
      return;
    }

    // 对时间取整, 不然会有如下错误:
    // 8点半系统启动，直到9点半，日志都会打到8点的文件里 
    next_rolling_time_ = now_rolling_time + rolling_period_;
  }

  // Add time prefix to log:20140711 01:20:22.325443
  struct timeval tv;
  gettimeofday(&tv, NULL);
  char buf[64];
  struct tm result;
  int off = strftime(buf, sizeof(buf), "%Y%m%d %H:%M:%S.",
                     localtime_r(&tv.tv_sec, &result));
  snprintf(buf+off, sizeof(buf)-off, "%06d", static_cast<int>(tv.tv_usec));
  fprintf(file_, "%s ", buf);

  if (vfprintf(file_, content, vl) < 0) {
    // Maybe disk is full
    fprintf(stderr, "Failed to append log file: %s, content:%s\n",
            filename_.c_str(), content);
  }
  // 不再 Should flush every time?
  // fflush(file_);
}

void LogFileObject::check_filesize() {
  if (file_ != NULL) {
    // in MB
    if ((ftell(file_) >> 20) >= max_log_size_) {
      fclose(file_);
      file_ = NULL;
    }
  }
}

void LogFileObject::check_rolling_time() {
  if (file_ != NULL) {
    timeval tv;
    gettimeofday(&tv, NULL);
    if (tv.tv_sec >= next_rolling_time_) {
      fclose(file_);
      file_ = NULL;
    }
  }
}

void LogFileObject::set_workable(bool workable) {
  // No status change
  if (workable_ == workable) {
    return;
  }

  // Disable workable logfileobject
  if (workable_) {
    fclose(file_);
    file_ = NULL;
  }
  workable_ = workable;
}

bool LogSystem::init(const std::string& logname, int max_log_size) {
  bool rv = true;
  rv &= debug_.init(logname, kLogLevelDebug, max_log_size);
  rv &= info_.init(logname, kLogLevelInfo, max_log_size);
  rv &= error_.init(logname, kLogLevelError, max_log_size);
  // rv &= data_.init(logname, kLogLevelData, max_log_size);
  return rv;
}

bool LogSystem::init_data_log(const std::string& logname, int max_log_size) {
  return data_.init(logname, kLogLevelData, max_log_size);
}

bool LogSystem::set_workable(const std::string& level, bool workable) {
  if (level == kLogLevelDebug) {
    debug_.set_workable(workable);
    return true;
  } else if (level == kLogLevelInfo) {
    info_.set_workable(workable);
    return true;
  } else if (level == kLogLevelError) {
    error_.set_workable(workable);
    return true;
  } else if (level == kLogLevelData) {
    data_.set_workable(workable);
    return true;
  } else {
    return false;
  }
}

void DEBUG_LOG(const char* content, ...) {
  va_list vl;
  va_start(vl, content);
  LogSystem::instance().debug().write(content, vl);
  va_end(vl);
}

void INFO_LOG(const char* content, ...) {
  va_list vl;
  va_start(vl, content);
  LogSystem::instance().info().write(content, vl);
  va_end(vl);
}

void ERROR_LOG(const char* content, ...) {
  va_list vl;
  va_start(vl, content);
  LogSystem::instance().error().write(content, vl);
  va_end(vl);
}

void DATA_LOG(const char* content, ...) {
  va_list vl;
  va_start(vl, content);
  LogSystem::instance().data().write(content, vl);
  va_end(vl);
}

void FlushLog() {
  LogSystem::instance().debug().flush();
  LogSystem::instance().info().flush();
  LogSystem::instance().error().flush();
  LogSystem::instance().data().flush();
}

void SigsegvHandler(int sig) {
  ERROR_LOG("Received SIGSEGV(%d) signal, flush all the logs immediately\n", sig);
  
  FlushLog(); 

  // 使用默认的处理函数
  struct sigaction sa;
  sa.sa_handler = SIG_DFL;
  sa.sa_flags = 0;
  sigaction(SIGSEGV, &sa, 0);

  raise(sig);
}

}  // namespace common
}  // namespace hrg
