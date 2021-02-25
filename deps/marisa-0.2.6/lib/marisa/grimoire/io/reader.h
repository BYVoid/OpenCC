#ifndef MARISA_GRIMOIRE_IO_READER_H_
#define MARISA_GRIMOIRE_IO_READER_H_

#include <cstdio>
#include <iostream>

#include "marisa/base.h"

namespace marisa {
namespace grimoire {
namespace io {

class Reader {
 public:
  Reader();
  ~Reader();

  void open(const char *filename);
  void open(std::FILE *file);
  void open(int fd);
  void open(std::istream &stream);

  template <typename T>
  void read(T *obj) {
    MARISA_THROW_IF(obj == NULL, MARISA_NULL_ERROR);
    read_data(obj, sizeof(T));
  }

  template <typename T>
  void read(T *objs, std::size_t num_objs) {
    MARISA_THROW_IF((objs == NULL) && (num_objs != 0), MARISA_NULL_ERROR);
    MARISA_THROW_IF(num_objs > (MARISA_SIZE_MAX / sizeof(T)),
        MARISA_SIZE_ERROR);
    read_data(objs, sizeof(T) * num_objs);
  }

  void seek(std::size_t size);

  bool is_open() const;

  void clear();
  void swap(Reader &rhs);

 private:
  std::FILE *file_;
  int fd_;
  std::istream *stream_;
  bool needs_fclose_;

  void open_(const char *filename);
  void open_(std::FILE *file);
  void open_(int fd);
  void open_(std::istream &stream);

  void read_data(void *buf, std::size_t size);

  // Disallows copy and assignment.
  Reader(const Reader &);
  Reader &operator=(const Reader &);
};

}  // namespace io
}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_IO_READER_H_
