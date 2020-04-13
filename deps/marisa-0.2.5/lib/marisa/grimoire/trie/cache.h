#ifndef MARISA_GRIMOIRE_TRIE_CACHE_H_
#define MARISA_GRIMOIRE_TRIE_CACHE_H_

#include <cfloat>

#include "marisa/base.h"

namespace marisa {
namespace grimoire {
namespace trie {

class Cache {
 public:
  Cache() : parent_(0), child_(0), union_() {
    union_.weight = FLT_MIN;
  }
  Cache(const Cache &cache)
      : parent_(cache.parent_), child_(cache.child_), union_(cache.union_) {}

  Cache &operator=(const Cache &cache) {
    parent_ = cache.parent_;
    child_ = cache.child_;
    union_ = cache.union_;
    return *this;
  }

  void set_parent(std::size_t parent) {
    MARISA_DEBUG_IF(parent > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    parent_ = (UInt32)parent;
  }
  void set_child(std::size_t child) {
    MARISA_DEBUG_IF(child > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    child_ = (UInt32)child;
  }
  void set_base(UInt8 base) {
    union_.link = (union_.link & ~0xFFU) | base;
  }
  void set_extra(std::size_t extra) {
    MARISA_DEBUG_IF(extra > (MARISA_UINT32_MAX >> 8), MARISA_SIZE_ERROR);
    union_.link = (UInt32)((union_.link & 0xFFU) | (extra << 8));
  }
  void set_weight(float weight) {
    union_.weight = weight;
  }

  std::size_t parent() const {
    return parent_;
  }
  std::size_t child() const {
    return child_;
  }
  UInt8 base() const {
    return (UInt8)(union_.link & 0xFFU);
  }
  std::size_t extra() const {
    return union_.link >> 8;
  }
  char label() const {
    return (char)base();
  }
  std::size_t link() const {
    return union_.link;
  }
  float weight() const {
    return union_.weight;
  }

 private:
  UInt32 parent_;
  UInt32 child_;
  union Union {
    UInt32 link;
    float weight;
  } union_;
};

}  // namespace trie
}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_TRIE_CACHE_H_
