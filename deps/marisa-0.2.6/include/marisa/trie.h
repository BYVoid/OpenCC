#ifndef MARISA_TRIE_H_
#define MARISA_TRIE_H_

#include "marisa/keyset.h"
#include "marisa/agent.h"

namespace marisa {
namespace grimoire {
namespace trie {

class LoudsTrie;

}  // namespace trie
}  // namespace grimoire

class Trie {
  friend class TrieIO;

 public:
  Trie();
  ~Trie();

  void build(Keyset &keyset, int config_flags = 0);

  void mmap(const char *filename);
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

  void clear();
  void swap(Trie &rhs);

 private:
  scoped_ptr<grimoire::trie::LoudsTrie> trie_;

  // Disallows copy and assignment.
  Trie(const Trie &);
  Trie &operator=(const Trie &);
};

}  // namespace marisa

#endif  // MARISA_TRIE_H_
