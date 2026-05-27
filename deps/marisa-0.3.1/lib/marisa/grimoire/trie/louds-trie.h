#ifndef MARISA_GRIMOIRE_TRIE_LOUDS_TRIE_H_
#define MARISA_GRIMOIRE_TRIE_LOUDS_TRIE_H_

#include <memory>

#include "marisa/agent.h"
#include "marisa/grimoire/trie/cache.h"
#include "marisa/grimoire/trie/config.h"
#include "marisa/grimoire/trie/key.h"
#include "marisa/grimoire/trie/tail.h"
#include "marisa/grimoire/vector.h"
#include "marisa/keyset.h"

namespace marisa::grimoire::trie {

class LoudsTrie {
 public:
  LoudsTrie();
  ~LoudsTrie();

  LoudsTrie(const LoudsTrie &) = delete;
  LoudsTrie &operator=(const LoudsTrie &) = delete;

  void build(Keyset &keyset, int flags);

  void map(Mapper &mapper);
  void read(Reader &reader);
  void write(Writer &writer) const;

  bool lookup(Agent &agent) const;
  void reverse_lookup(Agent &agent) const;
  bool common_prefix_search(Agent &agent) const;
  bool predictive_search(Agent &agent) const;

  std::size_t num_tries() const {
    return config_.num_tries();
  }
  std::size_t num_keys() const {
    return size();
  }
  std::size_t num_nodes() const {
    return (louds_.size() / 2) - 1;
  }

  CacheLevel cache_level() const {
    return config_.cache_level();
  }
  TailMode tail_mode() const {
    return config_.tail_mode();
  }
  NodeOrder node_order() const {
    return config_.node_order();
  }

  bool empty() const {
    return size() == 0;
  }
  std::size_t size() const {
    return terminal_flags_.num_1s();
  }
  std::size_t total_size() const;
  std::size_t io_size() const;

  void clear() noexcept;
  void swap(LoudsTrie &rhs) noexcept;

 private:
  BitVector louds_;
  BitVector terminal_flags_;
  BitVector link_flags_;
  Vector<uint8_t> bases_;
  FlatVector extras_;
  Tail tail_;
  std::unique_ptr<LoudsTrie> next_trie_;
  Vector<Cache> cache_;
  std::size_t cache_mask_ = 0;
  std::size_t num_l1_nodes_ = 0;
  Config config_;
  Mapper mapper_;

  void build_(Keyset &keyset, const Config &config);

  template <typename T>
  void build_trie(Vector<T> &keys, Vector<uint32_t> *terminals,
                  const Config &config, std::size_t trie_id);
  template <typename T>
  void build_current_trie(Vector<T> &keys, Vector<uint32_t> *terminals,
                          const Config &config, std::size_t trie_id);
  template <typename T>
  void build_next_trie(Vector<T> &keys, Vector<uint32_t> *terminals,
                       const Config &config, std::size_t trie_id);
  template <typename T>
  void build_terminals(const Vector<T> &keys,
                       Vector<uint32_t> *terminals) const;

  void reserve_cache(const Config &config, std::size_t trie_id,
                     std::size_t num_keys);
  template <typename T>
  void cache(std::size_t parent, std::size_t child, float weight, char label);
  void fill_cache();

  void map_(Mapper &mapper);
  void read_(Reader &reader);
  void write_(Writer &writer) const;

  inline bool find_child(Agent &agent) const;
  inline bool predictive_find_child(Agent &agent) const;

  inline void restore(Agent &agent, std::size_t node_id) const;
  inline bool match(Agent &agent, std::size_t node_id) const;
  inline bool prefix_match(Agent &agent, std::size_t node_id) const;

  void restore_(Agent &agent, std::size_t node_id) const;
  bool match_(Agent &agent, std::size_t node_id) const;
  bool prefix_match_(Agent &agent, std::size_t node_id) const;

  inline std::size_t get_cache_id(std::size_t node_id, char label) const;
  inline std::size_t get_cache_id(std::size_t node_id) const;

  inline std::size_t get_link(std::size_t node_id) const;
  inline std::size_t get_link(std::size_t node_id, std::size_t link_id) const;

  inline std::size_t update_link_id(std::size_t link_id,
                                    std::size_t node_id) const;
};

}  // namespace marisa::grimoire::trie

#endif  // MARISA_GRIMOIRE_TRIE_LOUDS_TRIE_H_
