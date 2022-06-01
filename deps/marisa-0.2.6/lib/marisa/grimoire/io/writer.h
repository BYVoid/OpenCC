#ifndef MARISA_GRIMOIRE_IO_WRITER_H_
#define MARISA_GRIMOIRE_IO_WRITER_H_

#include <cstdio>
#include <iostream>

#include "marisa/base.h"

namespace marisa {
namespace grimoire {
namespace io {

class Writer {
 public:
  Writer();
  ~Writer();

  void open(const char *filename);
  void open(std::FILE *file);
  void open(int fd);
  void open(std::ostream &stream);

  template <typename T>
  void write(const T &obj) {
    write_data(&obj, sizeof(T));
  }

  template <typename T>
  void write(const T *objs, std::size_t num_objs) {
    MARISA_THROW_IF((objs == NULL) && (num_objs != 0), MARISA_NULL_ERROR);
    MARISA_THROW_IF(num_objs > (MARISA_SIZE_MAX / sizeof(T)),
                    MARISA_SIZE_ERROR);
    write_data(objs, sizeof(T) * num_objs);
  }

  void seek(std::size_t size);

  bool is_open() const;

  void clear();
  void swap(Writer &rhs);

 private:
  std::FILE *file_;
  int fd_;
  std::ostream *stream_;
  bool needs_fclose_;

  void open_(const char *filename);
  void open_(std::FILE *file);
  void open_(int fd);
  void open_(std::ostream &stream);

  void write_data(const void *data, std::size_t size);

  // Disallows copy and assignment.
  Writer(const Writer &);
  Writer &operator=(const Writer &);
};

}  // namespace io
}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_IO_WRITER_H_
