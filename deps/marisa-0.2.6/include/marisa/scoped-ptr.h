#ifndef MARISA_SCOPED_PTR_H_
#define MARISA_SCOPED_PTR_H_

#include "marisa/base.h"

namespace marisa {

template <typename T>
class scoped_ptr {
 public:
  scoped_ptr() : ptr_(NULL) {}
  explicit scoped_ptr(T *ptr) : ptr_(ptr) {}

  ~scoped_ptr() {
    delete ptr_;
  }

  void reset(T *ptr = NULL) {
    MARISA_THROW_IF((ptr != NULL) && (ptr == ptr_), MARISA_RESET_ERROR);
    scoped_ptr(ptr).swap(*this);
  }

  T &operator*() const {
    MARISA_DEBUG_IF(ptr_ == NULL, MARISA_STATE_ERROR);
    return *ptr_;
  }
  T *operator->() const {
    MARISA_DEBUG_IF(ptr_ == NULL, MARISA_STATE_ERROR);
    return ptr_;
  }
  T *get() const {
    return ptr_;
  }

  void clear() {
    scoped_ptr().swap(*this);
  }
  void swap(scoped_ptr &rhs) {
    marisa::swap(ptr_, rhs.ptr_);
  }

 private:
  T *ptr_;

  // Disallows copy and assignment.
  scoped_ptr(const scoped_ptr &);
  scoped_ptr &operator=(const scoped_ptr &);
};

}  // namespace marisa

#endif  // MARISA_SCOPED_PTR_H_
