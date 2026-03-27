#ifndef MARISA_KEY_H_
#define MARISA_KEY_H_

#include <cassert>
#include <string_view>

#include "marisa/base.h"

namespace marisa {

class Key {
 public:
  Key() = default;
  Key(const Key &key) = default;
  Key &operator=(const Key &key) = default;

  char operator[](std::size_t i) const {
    assert(i < length_);
    return ptr_[i];
  }

  void set_str(std::string_view str) {
    set_str(str.data(), str.length());
  }
  void set_str(const char *str) {
    assert(str != nullptr);
    std::size_t length = 0;
    while (str[length] != '\0') {
      ++length;
    }
    assert(length <= UINT32_MAX);
    ptr_ = str;
    length_ = static_cast<uint32_t>(length);
  }
  void set_str(const char *ptr, std::size_t length) {
    assert((ptr != nullptr) || (length == 0));
    assert(length <= UINT32_MAX);
    ptr_ = ptr;
    length_ = static_cast<uint32_t>(length);
  }
  void set_id(std::size_t id) {
    assert(id <= UINT32_MAX);
    union_.id = static_cast<uint32_t>(id);
  }
  void set_weight(float weight) {
    union_.weight = weight;
  }

  std::string_view str() const {
    return std::string_view(ptr_, length_);
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

  void clear() noexcept {
    Key().swap(*this);
  }
  void swap(Key &rhs) noexcept {
    std::swap(ptr_, rhs.ptr_);
    std::swap(length_, rhs.length_);
    std::swap(union_.id, rhs.union_.id);
  }

 private:
  const char *ptr_ = nullptr;
  uint32_t length_ = 0;
  union Union {
    uint32_t id = 0;
    float weight;
  } union_;
};

}  // namespace marisa

#endif  // MARISA_KEY_H_
