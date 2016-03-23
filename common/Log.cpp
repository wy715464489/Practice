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

bool SplitLogname(const std::string& logname,
                  RULE_LIST* rules,
                  int* rolling_period) 
{
	size_t curr_pos = 0;
	size_t next_pos = 0;
	*rolling_period = kDefaultRollingPeriod;  // Default one day

	if(lgname.empty()) {
		return false;
	}

	while((next_pos = logname.find_first_of('%', curr_pos)) != std::string::npos) {
		if(next_pos != curr_pos) {
			rules->push_back(logname.substr(curr_pos, next_pos - curr_pos));
		}
		if ((next_pos + 1) == logname.size()) {
			return false;
		}
	}
	


}
