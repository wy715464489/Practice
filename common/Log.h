#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <vector>
#include <string>

typedef std::vector<std::string> RULE_LIST;

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

	// bool init(const std::string& logname,
	// 		  const std::string& level,
	// 		  int max_log_size);

	// void set_workable(bool workable);
	// void write(const char* content, ...);
 //  	void write(const char* content, va_list vl);

 //  	void check_filesize();
 //  	void check_rolling_time();

 //  	void flush() {
 //  		if( file_ != NULL )
 //  		{
 //  			fflush(file_);
 //  		}
 //  	}

  protected:

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