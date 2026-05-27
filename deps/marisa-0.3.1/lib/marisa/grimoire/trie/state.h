#ifndef MARISA_GRIMOIRE_TRIE_STATE_H_
#define MARISA_GRIMOIRE_TRIE_STATE_H_

#include <cassert>
#include <vector>

#include "marisa/grimoire/trie/history.h"

namespace marisa::grimoire::trie {

// A search agent has its internal state and the status codes are defined
// below.
enum StatusCode {
  MARISA_READY_TO_ALL,
  MARISA_READY_TO_COMMON_PREFIX_SEARCH,
  MARISA_READY_TO_PREDICTIVE_SEARCH,
  MARISA_END_OF_COMMON_PREFIX_SEARCH,
  MARISA_END_OF_PREDICTIVE_SEARCH,
};

class State {
 public:
  State() = default;

  State(const State &) = default;
  State &operator=(const State &) = default;
  State(State &&) noexcept = default;
  State &operator=(State &&) noexcept = default;

  void set_node_id(std::size_t node_id) {
    assert(node_id <= UINT32_MAX);
    node_id_ = static_cast<uint32_t>(node_id);
  }
  void set_query_pos(std::size_t query_pos) {
    assert(query_pos <= UINT32_MAX);
    query_pos_ = static_cast<uint32_t>(query_pos);
  }
  void set_history_pos(std::size_t history_pos) {
    assert(history_pos <= UINT32_MAX);
    history_pos_ = static_cast<uint32_t>(history_pos);
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

  const std::vector<char> &key_buf() const {
    return key_buf_;
  }
  const std::vector<History> &history() const {
    return history_;
  }

  std::vector<char> &key_buf() {
    return key_buf_;
  }
  std::vector<History> &history() {
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
  std::vector<char> key_buf_;
  std::vector<History> history_;
  uint32_t node_id_ = 0;
  uint32_t query_pos_ = 0;
  uint32_t history_pos_ = 0;
  StatusCode status_code_ = MARISA_READY_TO_ALL;
};

}  // namespace marisa::grimoire::trie

#endif  // MARISA_GRIMOIRE_TRIE_STATE_H_
