#include "gtest/gtest.h"
#include "common/Log.h"
#include <string>

using common::SplitLogname;
TEST(LogTest, SplitLogname) {
  using common::RULE_LIST;
  using common::kDefaultRollingPeriod;
  RULE_LIST rules;
  int rolling_period;
  bool rv = SplitLogname("", &rules, &rolling_period);
  EXPECT_FALSE(rv);
  EXPECT_TRUE(rules.empty()) << rules[0];
  EXPECT_TRUE(kDefaultRollingPeriod == rolling_period);
  // The minium time granularity is day
  RULE_LIST expect_rules;
  expect_rules.push_back("/tmp/log/");
  expect_rules.push_back("%y");
  expect_rules.push_back("-");
  expect_rules.push_back("%m");
  expect_rules.push_back("-");
  expect_rules.push_back("%d");
  expect_rules.push_back("/gamesrv.log");
  rv = SplitLogname("/tmp/log/%y-%m-%d/gamesrv.log", &rules, &rolling_period);
  EXPECT_TRUE(rv);
  EXPECT_TRUE(expect_rules == rules);
  EXPECT_TRUE(kDefaultRollingPeriod == rolling_period);

  expect_rules.clear();
  rules.clear();
  expect_rules.push_back("./");
  expect_rules.push_back("%y");
  expect_rules.push_back("-");
  expect_rules.push_back("%m");
  expect_rules.push_back("-");
  expect_rules.push_back("%d");
  expect_rules.push_back("/gamesrv-");
  expect_rules.push_back("%h");
  expect_rules.push_back(".log");
  rv = SplitLogname("./%y-%m-%d/gamesrv-%h.log",
                    &rules,
                    &rolling_period);
  EXPECT_TRUE(rv);
  EXPECT_TRUE(expect_rules == rules);
  EXPECT_EQ(60, rolling_period);

  // The minium time granularity is min, but rolling period is 5 minutes
  expect_rules.clear();
  rules.clear();
  expect_rules.push_back("/tmp/log/");
  expect_rules.push_back("%y");
  expect_rules.push_back("-");
  expect_rules.push_back("%m");
  expect_rules.push_back("-");
  expect_rules.push_back("%d");
  expect_rules.push_back("/gamesrv-");
  expect_rules.push_back("%h");
  expect_rules.push_back("-");
  expect_rules.push_back("%i");
  expect_rules.push_back(".log");
  rv = SplitLogname("/tmp/log/%y-%m-%d/gamesrv-%h-%5i.log",
                    &rules,
                    &rolling_period);
  EXPECT_TRUE(rv);
  EXPECT_TRUE(expect_rules == rules);
  EXPECT_EQ(5, rolling_period);
}

using common::ComposeLogfilename;
TEST(LogTest, ComposeLogfilename) {
  // Contain all rules: %y%m%d%h%i
  common::RULE_LIST rules;
  rules.push_back("/tmp/log/");
  rules.push_back("%y");
  rules.push_back("-");
  rules.push_back("%m");
  rules.push_back("-");
  rules.push_back("%d");
  rules.push_back("/gamesrv");
  rules.push_back("%h");
  rules.push_back("%i");
  const std::string level = "info";
  timeval tv;
  // Tue Jun 10 18:37:01 PDT 2014
  tv.tv_sec = 1402450623;
  tv.tv_usec = 318928;
  std::string filename;

  // gettimeofday(&tv, NULL);
  // time_t now_rolling_time = tv.tv_sec / rolling_period_ * rolling_period_;

  // ComposeLogfilename(rules, level, tv, 10, &filename);
  // const std::string expect_filename =
  //     "/tmp/log/2014-06-11/gamesrv0937.1402450623318928.info";
  // std::cout<< filename << std::endl;
  // EXPECT_TRUE(expect_filename == filename) << filename << "\t" << expect_filename;
}

using common::CreateFiledirectory;
TEST(LogTest, CreateFiledirectory) {
  std::string filename = "abcd/";
  bool rv = CreateFiledirectory(filename);
  EXPECT_TRUE(rv);
}

