#ifdef _WIN32
 #include <io.h>
#else  // _WIN32
 #include <unistd.h>
#endif  // _WIN32

#include <cerrno>
#include <limits>
#include <stdexcept>

#include "marisa/grimoire/io/reader.h"

namespace marisa::grimoire::io {

Reader::Reader() = default;

Reader::~Reader() {
  if (needs_fclose_) {
    std::fclose(file_);
  }
}

void Reader::open(const char *filename) {
  MARISA_THROW_IF(filename == nullptr, std::invalid_argument);

  Reader temp;
  temp.open_(filename);
  swap(temp);
}

void Reader::open(std::FILE *file) {
  MARISA_THROW_IF(file == nullptr, std::invalid_argument);

  Reader temp;
  temp.open_(file);
  swap(temp);
}

void Reader::open(int fd) {
  MARISA_THROW_IF(fd == -1, std::invalid_argument);

  Reader temp;
  temp.open_(fd);
  swap(temp);
}

void Reader::open(std::istream &stream) {
  Reader temp;
  temp.open_(stream);
  swap(temp);
}

void Reader::clear() noexcept {
  Reader().swap(*this);
}

void Reader::swap(Reader &rhs) noexcept {
  std::swap(file_, rhs.file_);
  std::swap(fd_, rhs.fd_);
  std::swap(stream_, rhs.stream_);
  std::swap(needs_fclose_, rhs.needs_fclose_);
}

void Reader::seek(std::size_t size) {
  MARISA_THROW_IF(!is_open(), std::logic_error);
  if (size == 0) {
    return;
  }
  if (size <= 16) {
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
  return (file_ != nullptr) || (fd_ != -1) || (stream_ != nullptr);
}

void Reader::open_(const char *filename) {
  std::FILE *file = nullptr;
#ifdef _MSC_VER
  const errno_t error_value = ::fopen_s(&file, filename, "rb");
  MARISA_THROW_SYSTEM_ERROR_IF(error_value != 0, error_value,
                               std::generic_category(), "fopen_s");
#else   // _MSC_VER
  file = std::fopen(filename, "rb");
  MARISA_THROW_SYSTEM_ERROR_IF(file == nullptr, errno, std::generic_category(),
                               "std::fopen");
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
  MARISA_THROW_IF(!is_open(), std::logic_error);
  if (size == 0) {
    return;
  }
  if (fd_ != -1) {
    while (size != 0) {
#ifdef _WIN32
      constexpr std::size_t CHUNK_SIZE = std::numeric_limits<int>::max();
      const unsigned int count = (size < CHUNK_SIZE) ? size : CHUNK_SIZE;
      const int size_read = ::_read(fd_, buf, count);
      MARISA_THROW_SYSTEM_ERROR_IF(size_read <= 0, errno,
                                   std::generic_category(), "_read");
#else   // _WIN32
      constexpr std::size_t CHUNK_SIZE = std::numeric_limits< ::ssize_t>::max();
      const ::size_t count = (size < CHUNK_SIZE) ? size : CHUNK_SIZE;
      const ::ssize_t size_read = ::read(fd_, buf, count);
      MARISA_THROW_SYSTEM_ERROR_IF(size_read <= 0, errno,
                                   std::generic_category(), "read");
#endif  // _WIN32
      buf = static_cast<char *>(buf) + size_read;
      size -= static_cast<std::size_t>(size_read);
    }
  } else if (file_ != nullptr) {
    MARISA_THROW_SYSTEM_ERROR_IF(std::fread(buf, 1, size, file_) != size, errno,
                                 std::generic_category(), "std::fread");
  } else if (stream_ != nullptr) {
    MARISA_THROW_IF(!stream_->read(static_cast<char *>(buf),
                                   static_cast<std::streamsize>(size)),
                    std::runtime_error);
  }
}

}  // namespace marisa::grimoire::io
