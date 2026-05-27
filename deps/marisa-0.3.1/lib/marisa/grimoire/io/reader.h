#ifndef MARISA_GRIMOIRE_IO_READER_H_
#define MARISA_GRIMOIRE_IO_READER_H_

#include <cstdio>
#include <iostream>
#include <stdexcept>

#include "marisa/base.h"

namespace marisa::grimoire::io {

class Reader {
 public:
  Reader();
  ~Reader();

  Reader(const Reader &) = delete;
  Reader &operator=(const Reader &) = delete;

  void open(const char *filename);
  void open(std::FILE *file);
  void open(int fd);
  void open(std::istream &stream);

  template <typename T>
  void read(T *obj) {
    MARISA_THROW_IF(obj == nullptr, std::invalid_argument);
    read_data(obj, sizeof(T));
  }

  template <typename T>
  void read(T *objs, std::size_t num_objs) {
    MARISA_THROW_IF((objs == nullptr) && (num_objs != 0),
                    std::invalid_argument);
    MARISA_THROW_IF(num_objs > (SIZE_MAX / sizeof(T)), std::invalid_argument);
    read_data(objs, sizeof(T) * num_objs);
  }

  void seek(std::size_t size);

  bool is_open() const;

  void clear() noexcept;
  void swap(Reader &rhs) noexcept;

 private:
  std::FILE *file_ = nullptr;
  int fd_ = -1;
  std::istream *stream_ = nullptr;
  bool needs_fclose_ = false;

  void open_(const char *filename);
  void open_(std::FILE *file);
  void open_(int fd);
  void open_(std::istream &stream);

  void read_data(void *buf, std::size_t size);
};

}  // namespace marisa::grimoire::io

#endif  // MARISA_GRIMOIRE_IO_READER_H_
