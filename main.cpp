// #include <sys/time.h>
// #include <sys/stat.h>
// #include <stdio.h>
// #include <stdarg.h>
// #include <stdint.h>
// #include <vector>
// #include <string>
// #include <errno.h>
// #include "gtest/gtest.h"

// using namespace std;

// bool CreateFiledirectory(const std::string& filename)
// {
// 	size_t pos = 0;
// 	while ((pos = filename.find_first_of('/', pos)) != std::string::npos) {
//     		pos += 1;
//     		const std::string filedir = filename.substr(0, pos);
//     		if (mkdir(filedir.c_str(), 0777) == -1) {
//                 if (errno != EEXIST) {
//         			std::string errno_msg = "Failed to make directory: ";
//         			errno_msg += filedir;
//         			perror(errno_msg.c_str());
//         			return false;
//       			}
//     		}
//   	}	
// 	return true;
// }

// int main()
// {
//     CreateFiledirectory("/home/chukie/develop/myTest/111/");
// 	printf("hello world");
// }
// Copyright [2012-2014] <HRG>
#include "gtest/gtest.h"
#include "common/Log.h"

GTEST_API_ int main(int argc, char **argv) {
  printf("Running main() from gtest_main.cc\n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

