#include <stdio.h>

#ifdef _WIN32
 #include <io.h>
#else  // _WIN32
 #include <unistd.h>
#endif  // _WIN32

#include <limits>

#include "marisa/grimoire/io/writer.h"

namespace marisa {
namespace grimoire {
namespace io {

Writer::Writer()
    : file_(NULL), fd_(-1), stream_(NULL), needs_fclose_(false) {}

Writer::~Writer() {
  if (needs_fclose_) {
    ::fclose(file_);
  }
}

void Writer::open(const char *filename) {
  MARISA_THROW_IF(filename == NULL, MARISA_NULL_ERROR);

  Writer temp;
  temp.open_(filename);
  swap(temp);
}

void Writer::open(std::FILE *file) {
  MARISA_THROW_IF(file == NULL, MARISA_NULL_ERROR);

  Writer temp;
  temp.open_(file);
  swap(temp);
}

void Writer::open(int fd) {
  MARISA_THROW_IF(fd == -1, MARISA_CODE_ERROR);

  Writer temp;
  temp.open_(fd);
  swap(temp);
}

void Writer::open(std::ostream &stream) {
  Writer temp;
  temp.open_(stream);
  swap(temp);
}

void Writer::clear() {
  Writer().swap(*this);
}

void Writer::swap(Writer &rhs) {
  marisa::swap(file_, rhs.file_);
  marisa::swap(fd_, rhs.fd_);
  marisa::swap(stream_, rhs.stream_);
  marisa::swap(needs_fclose_, rhs.needs_fclose_);
}

void Writer::seek(std::size_t size) {
  MARISA_THROW_IF(!is_open(), MARISA_STATE_ERROR);
  if (size == 0) {
    return;
  } else if (size <= 16) {
    const char buf[16] = {};
    write_data(buf, size);
  } else {
    const char buf[1024] = {};
    do {
      const std::size_t count = (size < sizeof(buf)) ? size : sizeof(buf);
      write_data(buf, count);
      size -= count;
    } while (size != 0);
  }
}

bool Writer::is_open() const {
  return (file_ != NULL) || (fd_ != -1) || (stream_ != NULL);
}

void Writer::open_(const char *filename) {
  std::FILE *file = NULL;
#ifdef _MSC_VER
  MARISA_THROW_IF(::fopen_s(&file, filename, "wb") != 0, MARISA_IO_ERROR);
#else  // _MSC_VER
  file = ::fopen(filename, "wb");
  MARISA_THROW_IF(file == NULL, MARISA_IO_ERROR);
#endif  // _MSC_VER
  file_ = file;
  needs_fclose_ = true;
}

void Writer::open_(std::FILE *file) {
  file_ = file;
}

void Writer::open_(int fd) {
  fd_ = fd;
}

void Writer::open_(std::ostream &stream) {
  stream_ = &stream;
}

void Writer::write_data(const void *data, std::size_t size) {
  MARISA_THROW_IF(!is_open(), MARISA_STATE_ERROR);
  if (size == 0) {
    return;
  } else if (fd_ != -1) {
    while (size != 0) {
#ifdef _WIN32
      static const std::size_t CHUNK_SIZE =
          std::numeric_limits<int>::max();
      const unsigned int count = (size < CHUNK_SIZE) ? size : CHUNK_SIZE;
      const int size_written = ::_write(fd_, data, count);
#else  // _WIN32
      static const std::size_t CHUNK_SIZE =
          std::numeric_limits< ::ssize_t>::max();
      const ::size_t count = (size < CHUNK_SIZE) ? size : CHUNK_SIZE;
      const ::ssize_t size_written = ::write(fd_, data, count);
#endif  // _WIN32
      MARISA_THROW_IF(size_written <= 0, MARISA_IO_ERROR);
      data = static_cast<const char *>(data) + size_written;
      size -= static_cast<std::size_t>(size_written);
    }
  } else if (file_ != NULL) {
    MARISA_THROW_IF(::fwrite(data, 1, size, file_) != size, MARISA_IO_ERROR);
    MARISA_THROW_IF(::fflush(file_) != 0, MARISA_IO_ERROR);
  } else if (stream_ != NULL) {
    try {
      MARISA_THROW_IF(!stream_->write(static_cast<const char *>(data),
          static_cast<std::streamsize>(size)), MARISA_IO_ERROR);
    } catch (const std::ios_base::failure &) {
      MARISA_THROW(MARISA_IO_ERROR, "std::ios_base::failure");
    }
  }
}

}  // namespace io
}  // namespace grimoire
}  // namespace marisa
