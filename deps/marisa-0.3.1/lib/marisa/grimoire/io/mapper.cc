#if (defined _WIN32) || (defined _WIN64)
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <windows.h>
#else  // (defined _WIN32) || (defined _WIN64)
 #include <fcntl.h>
 #include <sys/mman.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <unistd.h>
#endif  // (defined _WIN32) || (defined _WIN64)

#include <cerrno>
#include <stdexcept>

#include "marisa/grimoire/io/mapper.h"

namespace marisa::grimoire::io {

#if (defined _WIN32) || (defined _WIN64)
Mapper::Mapper() = default;
#else   // (defined _WIN32) || (defined _WIN64)
// mmap() returns MAP_FAILED on failure.
Mapper::Mapper() : origin_(MAP_FAILED) {}
#endif  // (defined _WIN32) || (defined _WIN64)

#if (defined _WIN32) || (defined _WIN64)
Mapper::~Mapper() {
  if (origin_ != nullptr) {
    ::UnmapViewOfFile(origin_);
  }

  if (map_ != nullptr) {
    ::CloseHandle(map_);
  }

  if (file_ != nullptr) {
    ::CloseHandle(file_);
  }
}
#else   // (defined _WIN32) || (defined _WIN64)
Mapper::~Mapper() {
  if (origin_ != MAP_FAILED) {
    ::munmap(origin_, size_);
  }

  if (fd_ != -1) {
    ::close(fd_);
  }
}
#endif  // (defined _WIN32) || (defined _WIN64)

void Mapper::open(const char *filename, int flags) {
  MARISA_THROW_IF(filename == nullptr, std::invalid_argument);

  Mapper temp;
  temp.open_(filename, flags);
  swap(temp);
}

void Mapper::open(const void *ptr, std::size_t size) {
  MARISA_THROW_IF((ptr == nullptr) && (size != 0), std::invalid_argument);

  Mapper temp;
  temp.open_(ptr, size);
  swap(temp);
}

void Mapper::seek(std::size_t size) {
  MARISA_THROW_IF(!is_open(), std::logic_error);
  MARISA_THROW_IF(size > avail_, std::runtime_error);

  map_data(size);
}

bool Mapper::is_open() const {
  return ptr_ != nullptr;
}

void Mapper::clear() noexcept {
  Mapper().swap(*this);
}

void Mapper::swap(Mapper &rhs) noexcept {
  std::swap(ptr_, rhs.ptr_);
  std::swap(avail_, rhs.avail_);
  std::swap(origin_, rhs.origin_);
  std::swap(size_, rhs.size_);
#if (defined _WIN32) || (defined _WIN64)
  std::swap(file_, rhs.file_);
  std::swap(map_, rhs.map_);
#else   // (defined _WIN32) || (defined _WIN64)
  std::swap(fd_, rhs.fd_);
#endif  // (defined _WIN32) || (defined _WIN64)
}

const void *Mapper::map_data(std::size_t size) {
  MARISA_THROW_IF(!is_open(), std::logic_error);
  MARISA_THROW_IF(size > avail_, std::runtime_error);

  const char *const data = static_cast<const char *>(ptr_);
  ptr_ = data + size;
  avail_ -= size;
  return data;
}

#if (defined _WIN32) || (defined _WIN64)
 #ifdef __MSVCRT_VERSION__
  #if __MSVCRT_VERSION__ >= 0x0601
   #define MARISA_HAS_STAT64
  #endif  // __MSVCRT_VERSION__ >= 0x0601
 #endif   // __MSVCRT_VERSION__
void Mapper::open_(const char *filename, int flags) {
  file_ = ::CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  MARISA_THROW_SYSTEM_ERROR_IF(file_ == INVALID_HANDLE_VALUE, ::GetLastError(),
                               std::system_category(), "CreateFileA");

  DWORD size_high, size_low;
  size_low = ::GetFileSize(file_, &size_high);
  MARISA_THROW_SYSTEM_ERROR_IF(size_low == INVALID_FILE_SIZE, ::GetLastError(),
                               std::system_category(), "GetFileSize");
  size_ = (std::size_t{size_high} << 32) | size_low;

  map_ = ::CreateFileMapping(file_, nullptr, PAGE_READONLY, 0, 0, nullptr);
  MARISA_THROW_SYSTEM_ERROR_IF(map_ == nullptr, ::GetLastError(),
                               std::system_category(), "CreateFileMapping");

  origin_ = ::MapViewOfFile(map_, FILE_MAP_READ, 0, 0, 0);
  MARISA_THROW_SYSTEM_ERROR_IF(origin_ == nullptr, ::GetLastError(),
                               std::system_category(), "MapViewOfFile");

  if (flags & MARISA_MAP_POPULATE) {
    WIN32_MEMORY_RANGE_ENTRY range_entry;
    range_entry.VirtualAddress = origin_;
    range_entry.NumberOfBytes = size_;
    ::PrefetchVirtualMemory(GetCurrentProcess(), 1, &range_entry, 0);
  }

  ptr_ = static_cast<const char *>(origin_);
  avail_ = size_;
}
#else  // (defined _WIN32) || (defined _WIN64)
void Mapper::open_(const char *filename, int flags) {
  fd_ = ::open(filename, O_RDONLY);
  MARISA_THROW_SYSTEM_ERROR_IF(fd_ == -1, errno, std::generic_category(),
                               "open");

  struct stat st;
  MARISA_THROW_SYSTEM_ERROR_IF(::fstat(fd_, &st) != 0, errno,
                               std::generic_category(), "fstat");
  MARISA_THROW_IF(static_cast<uint64_t>(st.st_size) > SIZE_MAX,
                  std::runtime_error);
  size_ = static_cast<std::size_t>(st.st_size);

  int map_flags = MAP_SHARED;
  if (flags & MARISA_MAP_POPULATE) {
 #if defined(MAP_POPULATE)
    // `MAP_POPULATE` is Linux-specific.
    map_flags |= MAP_POPULATE;
 #elif defined(MAP_PREFAULT_READ)
    // `MAP_PREFAULT_READ` is FreeBSD-specific.
    map_flags |= MAP_PREFAULT_READ;
 #endif
  }

  origin_ = ::mmap(nullptr, size_, PROT_READ, map_flags, fd_, 0);
  MARISA_THROW_SYSTEM_ERROR_IF(origin_ == MAP_FAILED, errno,
                               std::generic_category(), "mmap");

  ptr_ = static_cast<const char *>(origin_);
  avail_ = size_;
}
#endif  // (defined _WIN32) || (defined _WIN64)

void Mapper::open_(const void *ptr, std::size_t size) {
  ptr_ = ptr;
  avail_ = size;
}

}  // namespace marisa::grimoire::io
