#include "Log.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string>


namespace common {

bool SplitLogname(const std::string& logname, RULE_LIST* rules, int* rolling_period) {
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

void ComposeLogfilename(const RULE_LIST& rules, const std::string& level, const timeval& tv, time_t rolling_time, std::string* filename) {
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
	:	_workable(false),
		_has_inited(false),
		_level("info"),
		_max_log_size(1000),
		_file(NULL),
		_rolling_period(kDefaultRollingPeriod) {

}

bool LogFileObject::init(const std::string& logname, const std::string& level, int max_log_size) {
	if (_has_inited) {
		return true;
	}

	_level = level;
	_max_log_size = max_log_size;
	_rules.clear();

	if (!SplitLogname(logname, &_rules, &_rolling_period)) {
    //fprintf(stderr, "Wrong logname format: %s\n", logname.c_str());
    return false;
  }

  _rolling_period *= 60;
  _has_inited = true;
  return true;
}

LogFileObject::~LogFileObject() {
	if (_file != NULL) {
		fclose(_file);
		_file = NULL;
	}
}

void LogFileObject::write(const char* content, ...) {
	va_list vl;
	va_start(vl, content);
  write(content, vl);
  va_end(vl);
}

void LogFileObject::write(const char* content, va_list vl) {
	if (!_workable || !_has_inited) {
    return;
  }

  check_rolling_time();
  check_filesize();
}

void LogFileObject::check_filesize() {
  if (_file != NULL) {
    // in MB
    if ((ftell(_file) >> 20) >= _max_log_size) {
      fclose(_file);
      _file = NULL;
    }
  }
}

void LogFileObject::check_rolling_time() {
  if (_file != NULL) {
    timeval tv;
    gettimeofday(&tv, NULL);
    if (tv.tv_sec >= _next_rolling_time) {
      fclose(_file);
      _file = NULL;
    }
  }
}

}

