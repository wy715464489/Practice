#include "gtest/gtest.h"
#include "net/inet_address.h"
#include <tr1/functional>
#include "net/eventloop.h"
#include "net/channel.h"
#include <errno.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <queue>

// using net::CalculateTimer;

// TEST(InetAddressTest, IpAndPort)
// {
//   std::string ips[] = {
//     "0.0.0.0",
//     "10.10.2.1",
//     "127.0.0.0",
//     "192.168.1.1",
//     "225.1.33.11",
//     "255.255.255.255"
//   };

//   uint16_t ports[] = {
//     0,
//     10,
//     200,
//     500,
//     6000,
//     35661
//   };
//   EXPECT_EQ(sizeof(ips)/sizeof(ips[0]),
//           sizeof(ports)/sizeof(ports[0]));

//   for (size_t i = 0; i < sizeof(ips)/sizeof(ips[0]); i++) {
//     InetAddress addr;
//     EXPECT_TRUE(addr.init(ips[i], ports[i]));
//     EXPECT_EQ(ips[i], addr.ip());
//     EXPECT_EQ(ports[i], addr.port());
//   }

//   std::string invalid_ips[] = {"1234.4.2.1",
//                                 "100.123.0",
//                                 "a.b.c"
//                               };
//   for (size_t i = 0; i < sizeof(invalid_ips)/sizeof(invalid_ips[0]); i++) {
//     InetAddress addr;
//     EXPECT_FALSE(addr.init(invalid_ips[i],100));
//   }
// }

TEST(EventLoopTest, CalculateTimer) {
  // timeval tv;
  // tv.tv_sec = 1403570607;
  // tv.tv_usec = 465082;
  // time_t timer = CalculateTimer(tv);
  // EXPECT_EQ(static_cast<time_t>(1403570607) * 1000 + 465082 / 1000, timer);
}

// using net::Timer;
// using net::TimerComparer;
// using net::EventLoop;
// using net::Channel;
TEST(EventLoopTest, TimerComparer) {
  // typedef std::priority_queue<Timer, std::vector<Timer>, TimerComparer> TimerQueue;
  // TimerQueue timer_queue;

  // Timer t1;
  // t1.timer = 1002324;
  // timer_queue.push(t1);

  // Timer t2;
  // t2.timer = 10002323;
  // timer_queue.push(t2);

  // Timer t3;
  // t3.timer = 10002322;
  // timer_queue.push(t3);

  // Timer t4;
  // t4.timer = 1000;
  // timer_queue.push(t4);

  // Timer t5;
  // t5.timer = 100023230;
  // timer_queue.push(t5);

  // std::vector<Timer> expect_timers;
  // expect_timers.push_back(t4);
  // expect_timers.push_back(t1);
  // expect_timers.push_back(t3);
  // expect_timers.push_back(t2);
  // expect_timers.push_back(t5);

  // EXPECT_EQ(expect_timers.size(), timer_queue.size());
  // for (size_t i = 0; i < expect_timers.size(); i++) {
  //   Timer timer = timer_queue.top();
  //   EXPECT_EQ(expect_timers[i].timer, timer.timer);
  //   timer_queue.pop();
  // }
}

void TimerCallbackOne(int* num) {
  // (*num) += 1;
}

void TimerCallbackTwo(int* num) {
  // (*num) += 2;
}

TEST(EventLoopTest, Timer) {
  // typedef std::priority_queue<Timer, std::vector<Timer>, TimerComparer>
  //       TimerQueue;
  // TimerQueue timer_queue;

  // int num = 0;

  // Timer t1;
  // t1.timer = 10002324;
  // t1.cb = std::tr1::bind(TimerCallbackOne, &num);
  // timer_queue.push(t1);

  // Timer t2;
  // t2.timer = 10002323;
  // t2.cb = std::tr1::bind(TimerCallbackTwo, &num);
  // timer_queue.push(t2);

  // Timer timer = timer_queue.top();
  // timer.cb();
  // EXPECT_EQ(t2.timer, timer.timer);
  // EXPECT_EQ(2, num);

  // timer_queue.pop();
  // timer = timer_queue.top();
  // timer.cb();
  // EXPECT_EQ(t1.timer, timer.timer);
  // EXPECT_EQ(3, num);
}

// void EventCallBackUnitTest(int output_fd, Channel* channel, int* num) {
  // char c;
  // read(channel->fd(), &c, 1);  // pipe input
  // ++(*num);
  // write(output_fd, "c", 1);  // pipe output
  // if (*num == 5) {
  //   channel->disable_all();
  //   EventLoop::instance().quit();
  // }
// }


TEST(EventLoopTest, Loop) {
  // int file_pipe[2];
  // ASSERT_EQ(0, pipe(file_pipe));

  // Channel channel(EventLoop::instance().poller(), file_pipe[0]);
  // channel.enable_reading();
  // int read_num = 0;
  // channel.set_read_callback(std::tr1::bind(EventCallBackUnitTest,
  //                                         file_pipe[1],  // write
  //                                         &channel,
  //                                         &read_num));

  // EXPECT_EQ(1, write(file_pipe[1], "c", 1)) << strerror(errno);
  // EventLoop::instance().loop();
  // EXPECT_EQ(5, read_num);
}
