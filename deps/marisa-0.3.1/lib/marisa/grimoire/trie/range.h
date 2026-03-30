#ifndef MARISA_GRIMOIRE_TRIE_RANGE_H_
#define MARISA_GRIMOIRE_TRIE_RANGE_H_

#include <cassert>

#include "marisa/base.h"

namespace marisa::grimoire::trie {

class Range {
 public:
  Range() = default;

  void set_begin(std::size_t begin) {
    assert(begin <= UINT32_MAX);
    begin_ = static_cast<uint32_t>(begin);
  }
  void set_end(std::size_t end) {
    assert(end <= UINT32_MAX);
    end_ = static_cast<uint32_t>(end);
  }
  void set_key_pos(std::size_t key_pos) {
    assert(key_pos <= UINT32_MAX);
    key_pos_ = static_cast<uint32_t>(key_pos);
  }

  std::size_t begin() const {
    return begin_;
  }
  std::size_t end() const {
    return end_;
  }
  std::size_t key_pos() const {
    return key_pos_;
  }

 private:
  uint32_t begin_ = 0;
  uint32_t end_ = 0;
  uint32_t key_pos_ = 0;
};

inline Range make_range(std::size_t begin, std::size_t end,
                        std::size_t key_pos) {
  Range range;
  range.set_begin(begin);
  range.set_end(end);
  range.set_key_pos(key_pos);
  return range;
}

class WeightedRange {
 public:
  WeightedRange() = default;

  void set_range(const Range &range) {
    range_ = range;
  }
  void set_begin(std::size_t begin) {
    range_.set_begin(begin);
  }
  void set_end(std::size_t end) {
    range_.set_end(end);
  }
  void set_key_pos(std::size_t key_pos) {
    range_.set_key_pos(key_pos);
  }
  void set_weight(float weight) {
    weight_ = weight;
  }

  const Range &range() const {
    return range_;
  }
  std::size_t begin() const {
    return range_.begin();
  }
  std::size_t end() const {
    return range_.end();
  }
  std::size_t key_pos() const {
    return range_.key_pos();
  }
  float weight() const {
    return weight_;
  }

 private:
  Range range_;
  float weight_ = 0.0F;
};

inline bool operator<(const WeightedRange &lhs, const WeightedRange &rhs) {
  return lhs.weight() < rhs.weight();
}

inline bool operator>(const WeightedRange &lhs, const WeightedRange &rhs) {
  return lhs.weight() > rhs.weight();
}

inline WeightedRange make_weighted_range(std::size_t begin, std::size_t end,
                                         std::size_t key_pos, float weight) {
  WeightedRange range;
  range.set_begin(begin);
  range.set_end(end);
  range.set_key_pos(key_pos);
  range.set_weight(weight);
  return range;
}

}  // namespace marisa::grimoire::trie

#endif  // MARISA_GRIMOIRE_TRIE_RANGE_H_
