#ifndef MARISA_GRIMOIRE_TRIE_CONFIG_H_
#define MARISA_GRIMOIRE_TRIE_CONFIG_H_

#include <stdexcept>

#include "marisa/base.h"

namespace marisa::grimoire::trie {

class Config {
 public:
  Config() = default;

  Config(const Config &) = delete;
  Config &operator=(const Config &) = delete;

  void parse(int config_flags) {
    Config temp;
    temp.parse_(config_flags);
    swap(temp);
  }

  int flags() const {
    return static_cast<int>(num_tries_) | tail_mode_ | node_order_;
  }

  std::size_t num_tries() const {
    return num_tries_;
  }
  CacheLevel cache_level() const {
    return cache_level_;
  }
  TailMode tail_mode() const {
    return tail_mode_;
  }
  NodeOrder node_order() const {
    return node_order_;
  }

  void clear() noexcept {
    Config().swap(*this);
  }
  void swap(Config &rhs) noexcept {
    std::swap(num_tries_, rhs.num_tries_);
    std::swap(cache_level_, rhs.cache_level_);
    std::swap(tail_mode_, rhs.tail_mode_);
    std::swap(node_order_, rhs.node_order_);
  }

 private:
  std::size_t num_tries_ = MARISA_DEFAULT_NUM_TRIES;
  CacheLevel cache_level_ = MARISA_DEFAULT_CACHE;
  TailMode tail_mode_ = MARISA_DEFAULT_TAIL;
  NodeOrder node_order_ = MARISA_DEFAULT_ORDER;

  void parse_(int config_flags) {
    MARISA_THROW_IF((config_flags & ~MARISA_CONFIG_MASK) != 0,
                    std::invalid_argument);

    parse_num_tries(config_flags);
    parse_cache_level(config_flags);
    parse_tail_mode(config_flags);
    parse_node_order(config_flags);
  }

  void parse_num_tries(int config_flags) {
    const int num_tries = config_flags & MARISA_NUM_TRIES_MASK;
    if (num_tries != 0) {
      num_tries_ = static_cast<std::size_t>(num_tries);
    }
  }

  void parse_cache_level(int config_flags) {
    switch (config_flags & MARISA_CACHE_LEVEL_MASK) {
      case 0: {
        cache_level_ = MARISA_DEFAULT_CACHE;
        break;
      }
      case MARISA_HUGE_CACHE: {
        cache_level_ = MARISA_HUGE_CACHE;
        break;
      }
      case MARISA_LARGE_CACHE: {
        cache_level_ = MARISA_LARGE_CACHE;
        break;
      }
      case MARISA_NORMAL_CACHE: {
        cache_level_ = MARISA_NORMAL_CACHE;
        break;
      }
      case MARISA_SMALL_CACHE: {
        cache_level_ = MARISA_SMALL_CACHE;
        break;
      }
      case MARISA_TINY_CACHE: {
        cache_level_ = MARISA_TINY_CACHE;
        break;
      }
      default: {
        MARISA_THROW(std::invalid_argument, "undefined cache level");
      }
    }
  }

  void parse_tail_mode(int config_flags) {
    switch (config_flags & MARISA_TAIL_MODE_MASK) {
      case 0: {
        tail_mode_ = MARISA_DEFAULT_TAIL;
        break;
      }
      case MARISA_TEXT_TAIL: {
        tail_mode_ = MARISA_TEXT_TAIL;
        break;
      }
      case MARISA_BINARY_TAIL: {
        tail_mode_ = MARISA_BINARY_TAIL;
        break;
      }
      default: {
        MARISA_THROW(std::invalid_argument, "undefined tail mode");
      }
    }
  }

  void parse_node_order(int config_flags) {
    switch (config_flags & MARISA_NODE_ORDER_MASK) {
      case 0: {
        node_order_ = MARISA_DEFAULT_ORDER;
        break;
      }
      case MARISA_LABEL_ORDER: {
        node_order_ = MARISA_LABEL_ORDER;
        break;
      }
      case MARISA_WEIGHT_ORDER: {
        node_order_ = MARISA_WEIGHT_ORDER;
        break;
      }
      default: {
        MARISA_THROW(std::invalid_argument, "undefined node order");
      }
    }
  }
};

}  // namespace marisa::grimoire::trie

#endif  // MARISA_GRIMOIRE_TRIE_CONFIG_H_
