#ifndef MARISA_GRIMOIRE_TRIE_STATE_HISTORY_H_
#define MARISA_GRIMOIRE_TRIE_STATE_HISTORY_H_

#include <cassert>

#include "marisa/base.h"

namespace marisa::grimoire::trie {

class History {
 public:
  History() = default;

  void set_node_id(std::size_t node_id) {
    assert(node_id <= UINT32_MAX);
    node_id_ = static_cast<uint32_t>(node_id);
  }
  void set_louds_pos(std::size_t louds_pos) {
    assert(louds_pos <= UINT32_MAX);
    louds_pos_ = static_cast<uint32_t>(louds_pos);
  }
  void set_key_pos(std::size_t key_pos) {
    assert(key_pos <= UINT32_MAX);
    key_pos_ = static_cast<uint32_t>(key_pos);
  }
  void set_link_id(std::size_t link_id) {
    assert(link_id <= UINT32_MAX);
    link_id_ = static_cast<uint32_t>(link_id);
  }
  void set_key_id(std::size_t key_id) {
    assert(key_id <= UINT32_MAX);
    key_id_ = static_cast<uint32_t>(key_id);
  }

  std::size_t node_id() const {
    return node_id_;
  }
  std::size_t louds_pos() const {
    return louds_pos_;
  }
  std::size_t key_pos() const {
    return key_pos_;
  }
  std::size_t link_id() const {
    return link_id_;
  }
  std::size_t key_id() const {
    return key_id_;
  }

 private:
  uint32_t node_id_ = 0;
  uint32_t louds_pos_ = 0;
  uint32_t key_pos_ = 0;
  uint32_t link_id_ = MARISA_INVALID_LINK_ID;
  uint32_t key_id_ = MARISA_INVALID_KEY_ID;
};

}  // namespace marisa::grimoire::trie

#endif  // MARISA_GRIMOIRE_TRIE_STATE_HISTORY_H_
