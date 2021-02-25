#ifndef MARISA_GRIMOIRE_TRIE_STATE_HISTORY_H_
#define MARISA_GRIMOIRE_TRIE_STATE_HISTORY_H_

#include "marisa/base.h"

namespace marisa {
namespace grimoire {
namespace trie {

class History {
 public:
  History()
      : node_id_(0), louds_pos_(0), key_pos_(0),
        link_id_(MARISA_INVALID_LINK_ID), key_id_(MARISA_INVALID_KEY_ID) {}

  void set_node_id(std::size_t node_id) {
    MARISA_DEBUG_IF(node_id > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    node_id_ = (UInt32)node_id;
  }
  void set_louds_pos(std::size_t louds_pos) {
    MARISA_DEBUG_IF(louds_pos > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    louds_pos_ = (UInt32)louds_pos;
  }
  void set_key_pos(std::size_t key_pos) {
    MARISA_DEBUG_IF(key_pos > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    key_pos_ = (UInt32)key_pos;
  }
  void set_link_id(std::size_t link_id) {
    MARISA_DEBUG_IF(link_id > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    link_id_ = (UInt32)link_id;
  }
  void set_key_id(std::size_t key_id) {
    MARISA_DEBUG_IF(key_id > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    key_id_ = (UInt32)key_id;
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
  UInt32 node_id_;
  UInt32 louds_pos_;
  UInt32 key_pos_;
  UInt32 link_id_;
  UInt32 key_id_;
};

}  // namespace trie
}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_TRIE_STATE_HISTORY_H_
