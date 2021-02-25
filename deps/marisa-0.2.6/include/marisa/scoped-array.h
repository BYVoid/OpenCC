#ifndef MARISA_SCOPED_ARRAY_H_
#define MARISA_SCOPED_ARRAY_H_

#include "marisa/base.h"

namespace marisa {

template <typename T>
class scoped_array {
 public:
  scoped_array() : array_(NULL) {}
  explicit scoped_array(T *array) : array_(array) {}

  ~scoped_array() {
    delete [] array_;
  }

  void reset(T *array = NULL) {
    MARISA_THROW_IF((array != NULL) && (array == array_), MARISA_RESET_ERROR);
    scoped_array(array).swap(*this);
  }

  T &operator[](std::size_t i) const {
    MARISA_DEBUG_IF(array_ == NULL, MARISA_STATE_ERROR);
    return array_[i];
  }
  T *get() const {
    return array_;
  }

  void clear() {
    scoped_array().swap(*this);
  }
  void swap(scoped_array &rhs) {
    marisa::swap(array_, rhs.array_);
  }

 private:
  T *array_;

  // Disallows copy and assignment.
  scoped_array(const scoped_array &);
  scoped_array &operator=(const scoped_array &);
};

}  // namespace marisa

#endif  // MARISA_SCOPED_ARRAY_H_
