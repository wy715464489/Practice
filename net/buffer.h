// Copyright [2012-2014] <HRG>
#ifndef NET_BUFFER_H_
#define NET_BUFFER_H_

#include <cstddef>
#include <vector>
#include <algorithm>
#include "common/noncopyable.h"

// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
// Reference: http://docs.jboss.org/netty/3.2/api/org/jboss/netty/buffer/ChannelBuffer.html

//   +-------------------+------------------+------------------+
//   | discardable bytes |  readable bytes  |  writable bytes  |
//   |                   |     (CONTENT)    |                  |
//   +-------------------+------------------+------------------+
//   |                   |                  |                  |
//   0      <=     reader_index_  <=  writer_index_    <=    capacity

namespace hrg { namespace net {

class Buffer {
 public:
  static const size_t kInitialSize;
  static const size_t kBufferShrinkSizeThreshold;
  static const int    kBufferShrinkIdleThreshold;
  Buffer()
  : buffer_(kInitialSize),
    reader_index_(0),
    writer_index_(0) {
  }

  // On success,the number of bytes read is returned(zero indicates end of file)
  // On error, -1 is returned, and errno is set appropriately.
  int read_socket(int fd);

  // For buffer read
  size_t readable_bytes() const;

  const char* read_begin() const;

  void retrieve_all();
  void retrieve(size_t len);

  // For buffer write
  size_t writable_bytes() const;

  char* write_begin();

  void has_written(size_t len);

  // Make enough space to append len bytes
  void make_space(size_t len);

  void append(const char* data, size_t len);

  void shrink();

  // for unittest only
  const std::vector<char>& buffer() const;

 private:
  size_t discardable_bytes() const;

  char* begin();
  const char* begin() const;

  std::vector<char> buffer_;
  size_t reader_index_;
  size_t writer_index_;
  DISALLOW_COPY_AND_ASSIGN(Buffer);
};

}  // namespace net
}  // namespace hrg

#endif  // NET_BUFFER_H_
