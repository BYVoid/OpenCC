#ifndef MARISA_AGENT_H_
#define MARISA_AGENT_H_

#include <cassert>
#include <memory>
#include <string_view>

#include "marisa/key.h"
#include "marisa/query.h"

namespace marisa {
namespace grimoire::trie {

class State;

}  // namespace grimoire::trie

class Agent {
 public:
  Agent();
  ~Agent();

  Agent(const Agent &other);
  Agent &operator=(const Agent &other);
  Agent(Agent &&other) noexcept;
  Agent &operator=(Agent &&other) noexcept;

  const Query &query() const {
    return query_;
  }
  const Key &key() const {
    return key_;
  }

  void set_query(std::string_view str) {
    set_query(str.data(), str.length());
  }
  void set_query(const char *str);
  void set_query(const char *ptr, std::size_t length);
  void set_query(std::size_t key_id);

  const grimoire::trie::State &state() const {
    return *state_;
  }
  grimoire::trie::State &state() {
    return *state_;
  }

  void set_key(std::string_view str) {
    set_key(str.data(), str.length());
  }
  void set_key(const char *str) {
    assert(str != nullptr);
    key_.set_str(str);
  }
  void set_key(const char *ptr, std::size_t length) {
    assert((ptr != nullptr) || (length == 0));
    assert(length <= UINT32_MAX);
    key_.set_str(ptr, length);
  }
  void set_key(std::size_t id) {
    assert(id <= UINT32_MAX);
    key_.set_id(id);
  }

  bool has_state() const {
    return state_ != nullptr;
  }
  void init_state();

  void clear() noexcept;
  void swap(Agent &rhs) noexcept;

 private:
  Query query_;
  Key key_;
  std::unique_ptr<grimoire::trie::State> state_;
};

}  // namespace marisa

#endif  // MARISA_AGENT_H_
