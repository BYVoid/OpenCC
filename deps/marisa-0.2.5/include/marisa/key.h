#ifndef MARISA_KEY_H_
#define MARISA_KEY_H_

#include "marisa/base.h"

namespace marisa {

class Key {
 public:
  Key() : ptr_(NULL), length_(0), union_() {
    union_.id = 0;
  }
  Key(const Key &key)
      : ptr_(key.ptr_), length_(key.length_), union_(key.union_) {}

  Key &operator=(const Key &key) {
    ptr_ = key.ptr_;
    length_ = key.length_;
    union_ = key.union_;
    return *this;
  }

  char operator[](std::size_t i) const {
    MARISA_DEBUG_IF(i >= length_, MARISA_BOUND_ERROR);
    return ptr_[i];
  }

  void set_str(const char *str) {
    MARISA_DEBUG_IF(str == NULL, MARISA_NULL_ERROR);
    std::size_t length = 0;
    while (str[length] != '\0') {
      ++length;
    }
    MARISA_DEBUG_IF(length > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    ptr_ = str;
    length_ = (UInt32)length;
  }
  void set_str(const char *ptr, std::size_t length) {
    MARISA_DEBUG_IF((ptr == NULL) && (length != 0), MARISA_NULL_ERROR);
    MARISA_DEBUG_IF(length > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    ptr_ = ptr;
    length_ = (UInt32)length;
  }
  void set_id(std::size_t id) {
    MARISA_DEBUG_IF(id > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    union_.id = (UInt32)id;
  }
  void set_weight(float weight) {
    union_.weight = weight;
  }

  const char *ptr() const {
    return ptr_;
  }
  std::size_t length() const {
    return length_;
  }
  std::size_t id() const {
    return union_.id;
  }
  float weight() const {
    return union_.weight;
  }

  void clear() {
    Key().swap(*this);
  }
  void swap(Key &rhs) {
    marisa::swap(ptr_, rhs.ptr_);
    marisa::swap(length_, rhs.length_);
    marisa::swap(union_.id, rhs.union_.id);
  }

 private:
  const char *ptr_;
  UInt32 length_;
  union Union {
    UInt32 id;
    float weight;
  } union_;
};

}  // namespace marisa

#endif  // MARISA_KEY_H_
