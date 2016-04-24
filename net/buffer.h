#ifndef NET_BUFFER_H_
#define NET_BUFFER_H_

#include <cstddef>
#include <vector>
#include <algorithm>
#include "noncopyable.h"

class Buffer {
 public:
	static const size_t kInitialSize;
	static const size_t kBufferShrinkSizeThreshold;
	static const size_t kBufferShrinkIdleThreshold;
	Buffer()
	: _buffer(kInitialSize),
		_reader_index(0),
		_writer_index(0) {

	}

	int read_socket(int fd);

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
	
	std::vector<char> _buffer;
	size_t _reader_index;
	size_t _writer_index;
	DISALLOW_COPY_AND_ASSIGN(Buffer);
};
#endif