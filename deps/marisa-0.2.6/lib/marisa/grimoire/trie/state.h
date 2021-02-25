#ifndef MARISA_GRIMOIRE_TRIE_STATE_H_
#define MARISA_GRIMOIRE_TRIE_STATE_H_

#include "marisa/grimoire/vector.h"
#include "marisa/grimoire/trie/history.h"

namespace marisa {
namespace grimoire {
namespace trie {

// A search agent has its internal state and the status codes are defined
// below.
typedef enum StatusCode {
  MARISA_READY_TO_ALL,
  MARISA_READY_TO_COMMON_PREFIX_SEARCH,
  MARISA_READY_TO_PREDICTIVE_SEARCH,
  MARISA_END_OF_COMMON_PREFIX_SEARCH,
  MARISA_END_OF_PREDICTIVE_SEARCH,
} StatusCode;

class State {
 public:
  State()
      : key_buf_(), history_(), node_id_(0), query_pos_(0),
        history_pos_(0), status_code_(MARISA_READY_TO_ALL) {}

  void set_node_id(std::size_t node_id) {
    MARISA_DEBUG_IF(node_id > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    node_id_ = (UInt32)node_id;
  }
  void set_query_pos(std::size_t query_pos) {
    MARISA_DEBUG_IF(query_pos > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    query_pos_ = (UInt32)query_pos;
  }
  void set_history_pos(std::size_t history_pos) {
    MARISA_DEBUG_IF(history_pos > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    history_pos_ = (UInt32)history_pos;
  }
  void set_status_code(StatusCode status_code) {
    status_code_ = status_code;
  }

  std::size_t node_id() const {
    return node_id_;
  }
  std::size_t query_pos() const {
    return query_pos_;
  }
  std::size_t history_pos() const {
    return history_pos_;
  }
  StatusCode status_code() const {
    return status_code_;
  }

  const Vector<char> &key_buf() const {
    return key_buf_;
  }
  const Vector<History> &history() const {
    return history_;
  }

  Vector<char> &key_buf() {
    return key_buf_;
  }
  Vector<History> &history() {
    return history_;
  }

  void reset() {
    status_code_ = MARISA_READY_TO_ALL;
  }

  void lookup_init() {
    node_id_ = 0;
    query_pos_ = 0;
    status_code_ = MARISA_READY_TO_ALL;
  }
  void reverse_lookup_init() {
    key_buf_.resize(0);
    key_buf_.reserve(32);
    status_code_ = MARISA_READY_TO_ALL;
  }
  void common_prefix_search_init() {
    node_id_ = 0;
    query_pos_ = 0;
    status_code_ = MARISA_READY_TO_COMMON_PREFIX_SEARCH;
  }
  void predictive_search_init() {
    key_buf_.resize(0);
    key_buf_.reserve(64);
    history_.resize(0);
    history_.reserve(4);
    node_id_ = 0;
    query_pos_ = 0;
    history_pos_ = 0;
    status_code_ = MARISA_READY_TO_PREDICTIVE_SEARCH;
  }

 private:
  Vector<char> key_buf_;
  Vector<History> history_;
  UInt32 node_id_;
  UInt32 query_pos_;
  UInt32 history_pos_;
  StatusCode status_code_;

  // Disallows copy and assignment.
  State(const State &);
  State &operator=(const State &);
};

}  // namespace trie
}  // namespace grimoire
}  // namespace marisa

#endif  // MARISA_GRIMOIRE_TRIE_STATE_H_
