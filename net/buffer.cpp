#include "buffer.h"

const size_t Buffer::kInitialSize = 1024;
const size_t Buffer::kBufferShrinkSizeThreshold = 1024 * 100;
const size_t Buffer::kBufferShrinkIdleThreshold = 4;

int Buffer::read_socket(int fd) {
	static const size_t kWriteableBuffer = 512;

	int rv;
	int total_read = 0;

	do {
		if(writable_bytes() < kWriteableBuffer) {
			make_space(kWriteableBuffer);
		}

		do {
			rv = read(fd, write_begein(), writable_bytes());
		} while(rv == -1 && errno == EINTR);

		if (rv > 0) {
			total_read += rv;
			has_written(rv);
		}
	} while(rv > 0);

	if(total_read > 0)
		return total_read;
	else
		return rv;
}

size_t Buffer::readable_bytes() const {
	return _writer_index;
}

const char* Buffer::read_begin() const {
	return begin() + _read_index;
}

void Buffer::retrieve_all() {
	_read_index = 0;
	_writer_index = 0;
}

void Buffer::retrieve(size_t len) {
	if(len < readable_bytes()) {
		_read_index += len;
	}	 else {
		retrieve_all();
	}
}

size_t Buffer::writable_bytes() const {
  return _buffer.size() - _writer_index;
}

char* Buffer::write_begin() {
  return begin() + _writer_index;
}

void Buffer::has_written(size_t len) {
  _writer_index += len;
}

void Buffer::make_space(size_t len) {
	if(writable_bytes() + discardable_bytes() < len) {
		_buffer.resize(_writer_index + len);
	} else {
		const size_t num_bytes = readable_bytes();
		std::copy(begin() + _reader_index,
							begin() + _writer_index,
							begin());
		_reader_index = 0;
		_writer_index = num_bytes;
	}
}

void Buffer::append(const char* data, size_t len) {
  if (writable_bytes() < len)
    make_space(len);

  std::copy(data, data+len, write_begin());
  _writer_index += len;
}

void Buffer::shrink() {
  if (_buffer.size() >= kBufferShrinkSizeThreshold) {
    const size_t num_bytes = readable_bytes();
    if (num_bytes * kBufferShrinkIdleThreshold < buffer_.size()) {
      std::copy(begin()+_reader_index,
                begin()+_writer_index,
                begin());
      _reader_index = 0;
      _writer_index = num_bytes;
      _buffer.resize(num_bytes);
      std::vector<char>(_buffer).swap(_buffer);
    }
  }
}

size_t Buffer::discardable_bytes() const {
  return _reader_index;
}

char* Buffer::begin() {
  return &*_buffer.begin();
}

const char* Buffer::begin() const {
  return &*_buffer.begin();
}





size_t Buffer::writable_bytes() const {
  return _buffer.size() - _writer_index;
}
