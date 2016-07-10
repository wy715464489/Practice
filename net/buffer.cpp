// Copyright [2012-2014] <HRG>
#include "net/buffer.h"
#include <errno.h>
#include <unistd.h>
#include <algorithm>
#include <vector>

 namespace net {

// 1KB
const size_t Buffer::kInitialSize = 1024;
const size_t Buffer::kBufferShrinkSizeThreshold = 1024 * 100;
// maximin 75% idle
const int    Buffer::kBufferShrinkIdleThreshold = 4;

int Buffer::read_socket(int fd) {
  static const size_t kWriteableBuffer = 512;
  // As similar as Apache apr library
  int rv;
  int total_read = 0;
  do {
      // Make read effective
      if (writable_bytes() < kWriteableBuffer) {
        make_space(kWriteableBuffer);
      }

      do {
        rv = read(fd, write_begin(), writable_bytes());
      } while (rv == -1 && errno == EINTR);

      if (rv > 0) {
        total_read += rv;
        has_written(rv);
      }
  } while (rv > 0);

  if (total_read > 0)
    return total_read;
  else
    return rv;
}

// For buffer read
size_t Buffer::readable_bytes() const {
  return writer_index_ - reader_index_;
}

const char* Buffer::read_begin() const {
  return begin() + reader_index_;
}

void Buffer::retrieve_all() {
  reader_index_ = 0;
  writer_index_ = 0;
}

void Buffer::retrieve(size_t len) {
  if (len < readable_bytes()) {
    reader_index_ += len;
  } else {
    retrieve_all();
  }
}

size_t Buffer::writable_bytes() const {
  return buffer_.size() - writer_index_;
}

char* Buffer::write_begin() {
  return begin() + writer_index_;
}

void Buffer::has_written(size_t len) {
  writer_index_ += len;
}

void Buffer::make_space(size_t len) {
  if (writable_bytes() + discardable_bytes() < len) {
    buffer_.resize(writer_index_ + len);
  } else {
    const size_t num_bytes = readable_bytes();
    std::copy(begin()+reader_index_,
              begin()+writer_index_,
              begin());
    reader_index_ = 0;
    writer_index_ = num_bytes;
  }
}

void Buffer::append(const char* data, size_t len) {
  if (writable_bytes() < len)
    make_space(len);

  std::copy(data, data+len, write_begin());
  writer_index_ += len;
}


void Buffer::shrink() {
  if (buffer_.size() >= kBufferShrinkSizeThreshold) {
    const size_t num_bytes = readable_bytes();
    if (num_bytes * kBufferShrinkIdleThreshold < buffer_.size()) {
      std::copy(begin()+reader_index_,
                begin()+writer_index_,
                begin());
      reader_index_ = 0;
      writer_index_ = num_bytes;
      buffer_.resize(num_bytes);
      std::vector<char>(buffer_).swap(buffer_);
    }
  }
}

size_t Buffer::discardable_bytes() const {
  return reader_index_;
}

char* Buffer::begin() {
  return &*buffer_.begin();
}

const char* Buffer::begin() const {
  return &*buffer_.begin();
}

const std::vector<char>& Buffer::buffer() const {
  return buffer_;
}

}  // namespace net

