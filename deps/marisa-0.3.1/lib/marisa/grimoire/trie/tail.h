#ifndef MARISA_GRIMOIRE_TRIE_TAIL_H_
#define MARISA_GRIMOIRE_TRIE_TAIL_H_

#include <cassert>

#include "marisa/agent.h"
#include "marisa/grimoire/trie/entry.h"
#include "marisa/grimoire/vector.h"

namespace marisa::grimoire::trie {

class Tail {
 public:
  Tail();

  Tail(const Tail &) = delete;
  Tail &operator=(const Tail &) = delete;

  void build(Vector<Entry> &entries, Vector<uint32_t> *offsets, TailMode mode);

  void map(Mapper &mapper);
  void read(Reader &reader);
  void write(Writer &writer) const;

  void restore(Agent &agent, std::size_t offset) const;
  bool match(Agent &agent, std::size_t offset) const;
  bool prefix_match(Agent &agent, std::size_t offset) const;

  const char &operator[](std::size_t offset) const {
    assert(offset < buf_.size());
    return buf_[offset];
  }

  TailMode mode() const {
    return end_flags_.empty() ? MARISA_TEXT_TAIL : MARISA_BINARY_TAIL;
  }

  bool empty() const {
    return buf_.empty();
  }
  std::size_t size() const {
    return buf_.size();
  }
  std::size_t total_size() const {
    return buf_.total_size() + end_flags_.total_size();
  }
  std::size_t io_size() const {
    return buf_.io_size() + end_flags_.io_size();
  }

  void clear() noexcept;
  void swap(Tail &rhs) noexcept;

 private:
  Vector<char> buf_;
  BitVector end_flags_;

  void build_(Vector<Entry> &entries, Vector<uint32_t> *offsets, TailMode mode);

  void map_(Mapper &mapper);
  void read_(Reader &reader);
  void write_(Writer &writer) const;
};

}  // namespace marisa::grimoire::trie

#endif  // MARISA_GRIMOIRE_TRIE_TAIL_H_
