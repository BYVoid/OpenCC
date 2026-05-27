#ifndef MARISA_TRIE_H_
#define MARISA_TRIE_H_

#include <memory>

#include "marisa/agent.h"   // IWYU pragma: export
#include "marisa/keyset.h"  // IWYU pragma: export

namespace marisa {
namespace grimoire::trie {

class LoudsTrie;

}  // namespace grimoire::trie

class Trie {
  friend class TrieIO;

 public:
  Trie();
  ~Trie();

  Trie(const Trie &) = delete;
  Trie &operator=(const Trie &) = delete;

  Trie(Trie &&) noexcept;
  Trie &operator=(Trie &&) noexcept;

  void build(Keyset &keyset, int config_flags = 0);

  void mmap(const char *filename, int flags = 0);
  void map(const void *ptr, std::size_t size);

  void load(const char *filename);
  void read(int fd);

  void save(const char *filename) const;
  void write(int fd) const;

  bool lookup(Agent &agent) const;
  void reverse_lookup(Agent &agent) const;
  bool common_prefix_search(Agent &agent) const;
  bool predictive_search(Agent &agent) const;

  std::size_t num_tries() const;
  std::size_t num_keys() const;
  std::size_t num_nodes() const;

  TailMode tail_mode() const;
  NodeOrder node_order() const;

  bool empty() const;
  std::size_t size() const;
  std::size_t total_size() const;
  std::size_t io_size() const;

  void clear() noexcept;
  void swap(Trie &rhs) noexcept;

 private:
  std::unique_ptr<grimoire::trie::LoudsTrie> trie_;
};

}  // namespace marisa

#endif  // MARISA_TRIE_H_
