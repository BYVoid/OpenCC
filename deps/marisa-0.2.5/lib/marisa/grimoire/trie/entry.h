#ifndef MARISA_GRIMOIRE_TRIE_ENTRY_H_
#define MARISA_GRIMOIRE_TRIE_ENTRY_H_

#include "marisa/base.h"

namespace marisa {
namespace grimoire {
namespace trie {

class Entry {
 public:
  Entry()
      : ptr_(static_cast<const char *>(NULL) - 1), length_(0), id_(0) {}
  Entry(const Entry &entry)
      : ptr_(entry.ptr_), length_(entry.length_), id_(entry.id_) {}

  Entry &operator=(const Entry &entry) {
    ptr_ = entry.ptr_;
    length_ = entry.length_;
    id_ = entry.id_;
    return *this;
  }

  char operator[](std::size_t i) const {
    MARISA_DEBUG_IF(i >= length_, MARISA_BOUND_ERROR);
    return *(ptr_ - i);
  }

  void set_str(const char *ptr, std::size_t length) {
    MARISA_DEBUG_IF((ptr == NULL) && (length != 0), MARISA_NULL_ERROR);
    MARISA_DEBUG_IF(length > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    ptr_ = ptr + length - 1;
    length_ = (UInt32)length;
  }
  void set_id(std::size_t id) {
    MARISA_DEBUG_IF(id > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    id_ = (UInt32)id;
  }

  const char *ptr() const {
    return ptr_ - length_ + 1;
  }
  std::size_t length() const {
    return length_;
  }
  std::size_t id() const {
    return id_;
  }

  class StringComparer {
   public:
    bool operator()(const Entry &lhs, const Entry &rhs) const {
      for (std::size_t i = 0; i < lhs.length(); ++i) {
        if (i == rhs.length()) {
          return true;
        }
        if (lhs[i] != rhs[i]) {
          return (UInt8)lhs[i] > (UInt8)rhs[i];
        }
      }
      return lhs.length() > rhs.length();
    }
  };

  class IDComparer {
   public:
    bool operator()(const Entry &lhs, const Entry &rhs) const {
      return lhs.id_ < rhs.id_;
    }
  };

 private:
  const char *ptr_;
  UInt32 length_;
  UInt32 id_;
};

}  // namespace trie
}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_TRIE_ENTRY_H_
