#ifndef MARISA_GRIMOIRE_TRIE_KEY_H_
#define MARISA_GRIMOIRE_TRIE_KEY_H_

#include "marisa/base.h"

namespace marisa {
namespace grimoire {
namespace trie {

class Key {
 public:
  Key() : ptr_(NULL), length_(0), union_(), id_(0) {
    union_.terminal = 0;
  }
  Key(const Key &entry)
      : ptr_(entry.ptr_), length_(entry.length_),
        union_(entry.union_), id_(entry.id_) {}

  Key &operator=(const Key &entry) {
    ptr_ = entry.ptr_;
    length_ = entry.length_;
    union_ = entry.union_;
    id_ = entry.id_;
    return *this;
  }

  char operator[](std::size_t i) const {
    MARISA_DEBUG_IF(i >= length_, MARISA_BOUND_ERROR);
    return ptr_[i];
  }

  void substr(std::size_t pos, std::size_t length) {
    MARISA_DEBUG_IF(pos > length_, MARISA_BOUND_ERROR);
    MARISA_DEBUG_IF(length > length_, MARISA_BOUND_ERROR);
    MARISA_DEBUG_IF(pos > (length_ - length), MARISA_BOUND_ERROR);
    ptr_ += pos;
    length_ = (UInt32)length;
  }

  void set_str(const char *ptr, std::size_t length) {
    MARISA_DEBUG_IF((ptr == NULL) && (length != 0), MARISA_NULL_ERROR);
    MARISA_DEBUG_IF(length > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    ptr_ = ptr;
    length_ = (UInt32)length;
  }
  void set_weight(float weight) {
    union_.weight = weight;
  }
  void set_terminal(std::size_t terminal) {
    MARISA_DEBUG_IF(terminal > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    union_.terminal = (UInt32)terminal;
  }
  void set_id(std::size_t id) {
    MARISA_DEBUG_IF(id > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    id_ = (UInt32)id;
  }

  const char *ptr() const {
    return ptr_;
  }
  std::size_t length() const {
    return length_;
  }
  float weight() const {
    return union_.weight;
  }
  std::size_t terminal() const {
    return union_.terminal;
  }
  std::size_t id() const {
    return id_;
  }

 private:
  const char *ptr_;
  UInt32 length_;
  union Union {
    float weight;
    UInt32 terminal;
  } union_;
  UInt32 id_;
};

inline bool operator==(const Key &lhs, const Key &rhs) {
  if (lhs.length() != rhs.length()) {
    return false;
  }
  for (std::size_t i = 0; i < lhs.length(); ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

inline bool operator!=(const Key &lhs, const Key &rhs) {
  return !(lhs == rhs);
}

inline bool operator<(const Key &lhs, const Key &rhs) {
  for (std::size_t i = 0; i < lhs.length(); ++i) {
    if (i == rhs.length()) {
      return false;
    }
    if (lhs[i] != rhs[i]) {
      return (UInt8)lhs[i] < (UInt8)rhs[i];
    }
  }
  return lhs.length() < rhs.length();
}

inline bool operator>(const Key &lhs, const Key &rhs) {
  return rhs < lhs;
}

class ReverseKey {
 public:
  ReverseKey() : ptr_(NULL), length_(0), union_(), id_(0) {
    union_.terminal = 0;
  }
  ReverseKey(const ReverseKey &entry)
      : ptr_(entry.ptr_), length_(entry.length_),
        union_(entry.union_), id_(entry.id_) {}

  ReverseKey &operator=(const ReverseKey &entry) {
    ptr_ = entry.ptr_;
    length_ = entry.length_;
    union_ = entry.union_;
    id_ = entry.id_;
    return *this;
  }

  char operator[](std::size_t i) const {
    MARISA_DEBUG_IF(i >= length_, MARISA_BOUND_ERROR);
    return *(ptr_ - i - 1);
  }

  void substr(std::size_t pos, std::size_t length) {
    MARISA_DEBUG_IF(pos > length_, MARISA_BOUND_ERROR);
    MARISA_DEBUG_IF(length > length_, MARISA_BOUND_ERROR);
    MARISA_DEBUG_IF(pos > (length_ - length), MARISA_BOUND_ERROR);
    ptr_ -= pos;
    length_ = (UInt32)length;
  }

  void set_str(const char *ptr, std::size_t length) {
    MARISA_DEBUG_IF((ptr == NULL) && (length != 0), MARISA_NULL_ERROR);
    MARISA_DEBUG_IF(length > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    ptr_ = ptr + length;
    length_ = (UInt32)length;
  }
  void set_weight(float weight) {
    union_.weight = weight;
  }
  void set_terminal(std::size_t terminal) {
    MARISA_DEBUG_IF(terminal > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    union_.terminal = (UInt32)terminal;
  }
  void set_id(std::size_t id) {
    MARISA_DEBUG_IF(id > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    id_ = (UInt32)id;
  }

  const char *ptr() const {
    return ptr_ - length_;
  }
  std::size_t length() const {
    return length_;
  }
  float weight() const {
    return union_.weight;
  }
  std::size_t terminal() const {
    return union_.terminal;
  }
  std::size_t id() const {
    return id_;
  }

 private:
  const char *ptr_;
  UInt32 length_;
  union Union {
    float weight;
    UInt32 terminal;
  } union_;
  UInt32 id_;
};

inline bool operator==(const ReverseKey &lhs, const ReverseKey &rhs) {
  if (lhs.length() != rhs.length()) {
    return false;
  }
  for (std::size_t i = 0; i < lhs.length(); ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

inline bool operator!=(const ReverseKey &lhs, const ReverseKey &rhs) {
  return !(lhs == rhs);
}

inline bool operator<(const ReverseKey &lhs, const ReverseKey &rhs) {
  for (std::size_t i = 0; i < lhs.length(); ++i) {
    if (i == rhs.length()) {
      return false;
    }
    if (lhs[i] != rhs[i]) {
      return (UInt8)lhs[i] < (UInt8)rhs[i];
    }
  }
  return lhs.length() < rhs.length();
}

inline bool operator>(const ReverseKey &lhs, const ReverseKey &rhs) {
  return rhs < lhs;
}

}  // namespace trie
}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_TRIE_KEY_H_
