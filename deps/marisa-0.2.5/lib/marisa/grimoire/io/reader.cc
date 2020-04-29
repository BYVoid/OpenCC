#include <stdio.h>

#ifdef _WIN32
 #include <io.h>
#else  // _WIN32
 #include <unistd.h>
#endif  // _WIN32

#include <limits>

#include "marisa/grimoire/io/reader.h"

namespace marisa {
namespace grimoire {
namespace io {

Reader::Reader()
    : file_(NULL), fd_(-1), stream_(NULL), needs_fclose_(false) {}

Reader::~Reader() {
  if (needs_fclose_) {
    ::fclose(file_);
  }
}

void Reader::open(const char *filename) {
  MARISA_THROW_IF(filename == NULL, MARISA_NULL_ERROR);

  Reader temp;
  temp.open_(filename);
  swap(temp);
}

void Reader::open(std::FILE *file) {
  MARISA_THROW_IF(file == NULL, MARISA_NULL_ERROR);

  Reader temp;
  temp.open_(file);
  swap(temp);
}

void Reader::open(int fd) {
  MARISA_THROW_IF(fd == -1, MARISA_CODE_ERROR);

  Reader temp;
  temp.open_(fd);
  swap(temp);
}

void Reader::open(std::istream &stream) {
  Reader temp;
  temp.open_(stream);
  swap(temp);
}

void Reader::clear() {
  Reader().swap(*this);
}

void Reader::swap(Reader &rhs) {
  marisa::swap(file_, rhs.file_);
  marisa::swap(fd_, rhs.fd_);
  marisa::swap(stream_, rhs.stream_);
  marisa::swap(needs_fclose_, rhs.needs_fclose_);
}

void Reader::seek(std::size_t size) {
  MARISA_THROW_IF(!is_open(), MARISA_STATE_ERROR);
  if (size == 0) {
    return;
  } else if (size <= 16) {
    char buf[16];
    read_data(buf, size);
  } else {
    char buf[1024];
    while (size != 0) {
      const std::size_t count = (size < sizeof(buf)) ? size : sizeof(buf);
      read_data(buf, count);
      size -= count;
    }
  }
}

bool Reader::is_open() const {
  return (file_ != NULL) || (fd_ != -1) || (stream_ != NULL);
}

void Reader::open_(const char *filename) {
  std::FILE *file = NULL;
#ifdef _MSC_VER
  MARISA_THROW_IF(::fopen_s(&file, filename, "rb") != 0, MARISA_IO_ERROR);
#else  // _MSC_VER
  file = ::fopen(filename, "rb");
  MARISA_THROW_IF(file == NULL, MARISA_IO_ERROR);
#endif  // _MSC_VER
  file_ = file;
  needs_fclose_ = true;
}

void Reader::open_(std::FILE *file) {
  file_ = file;
}

void Reader::open_(int fd) {
  fd_ = fd;
}

void Reader::open_(std::istream &stream) {
  stream_ = &stream;
}

void Reader::read_data(void *buf, std::size_t size) {
  MARISA_THROW_IF(!is_open(), MARISA_STATE_ERROR);
  if (size == 0) {
    return;
  } else if (fd_ != -1) {
    while (size != 0) {
#ifdef _WIN32
      static const std::size_t CHUNK_SIZE =
          std::numeric_limits<int>::max();
      const unsigned int count = (size < CHUNK_SIZE) ? size : CHUNK_SIZE;
      const int size_read = ::_read(fd_, buf, count);
#else  // _WIN32
      static const std::size_t CHUNK_SIZE =
          std::numeric_limits< ::ssize_t>::max();
      const ::size_t count = (size < CHUNK_SIZE) ? size : CHUNK_SIZE;
      const ::ssize_t size_read = ::read(fd_, buf, count);
#endif  // _WIN32
      MARISA_THROW_IF(size_read <= 0, MARISA_IO_ERROR);
      buf = static_cast<char *>(buf) + size_read;
      size -= size_read;
    }
  } else if (file_ != NULL) {
    MARISA_THROW_IF(::fread(buf, 1, size, file_) != size, MARISA_IO_ERROR);
  } else if (stream_ != NULL) {
    try {
      MARISA_THROW_IF(!stream_->read(static_cast<char *>(buf), size),
          MARISA_IO_ERROR);
    } catch (const std::ios_base::failure &) {
      MARISA_THROW(MARISA_IO_ERROR, "std::ios_base::failure");
    }
  }
}

}  // namespace io
}  // namespace grimoire
}  // namespace marisa
