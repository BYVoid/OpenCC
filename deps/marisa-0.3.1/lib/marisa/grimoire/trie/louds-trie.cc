#include "marisa/grimoire/trie/louds-trie.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <queue>
#include <stdexcept>

#include "marisa/grimoire/algorithm/sort.h"
#include "marisa/grimoire/trie/header.h"
#include "marisa/grimoire/trie/range.h"
#include "marisa/grimoire/trie/state.h"

namespace marisa::grimoire::trie {

LoudsTrie::LoudsTrie() = default;

LoudsTrie::~LoudsTrie() = default;

void LoudsTrie::build(Keyset &keyset, int flags) {
  Config config;
  config.parse(flags);

  LoudsTrie temp;
  temp.build_(keyset, config);
  swap(temp);
}

void LoudsTrie::map(Mapper &mapper) {
  Header().map(mapper);

  LoudsTrie temp;
  temp.map_(mapper);
  temp.mapper_.swap(mapper);
  swap(temp);
}

void LoudsTrie::read(Reader &reader) {
  Header().read(reader);

  LoudsTrie temp;
  temp.read_(reader);
  swap(temp);
}

void LoudsTrie::write(Writer &writer) const {
  Header().write(writer);

  write_(writer);
}

bool LoudsTrie::lookup(Agent &agent) const {
  assert(agent.has_state());

  State &state = agent.state();
  state.lookup_init();
  while (state.query_pos() < agent.query().length()) {
    if (!find_child(agent)) {
      return false;
    }
  }
  if (!terminal_flags_[state.node_id()]) {
    return false;
  }
  agent.set_key(agent.query().ptr(), agent.query().length());
  agent.set_key(terminal_flags_.rank1(state.node_id()));
  return true;
}

void LoudsTrie::reverse_lookup(Agent &agent) const {
  assert(agent.has_state());
  MARISA_THROW_IF(agent.query().id() >= size(), std::out_of_range);

  State &state = agent.state();
  state.reverse_lookup_init();

  state.set_node_id(terminal_flags_.select1(agent.query().id()));
  if (state.node_id() == 0) {
    agent.set_key(state.key_buf().data(), state.key_buf().size());
    agent.set_key(agent.query().id());
    return;
  }
  for (;;) {
    if (link_flags_[state.node_id()]) {
      const std::size_t prev_key_pos = state.key_buf().size();
      restore(agent, get_link(state.node_id()));
      std::reverse(
          state.key_buf().begin() + static_cast<ptrdiff_t>(prev_key_pos),
          state.key_buf().end());
    } else {
      state.key_buf().push_back(static_cast<char>(bases_[state.node_id()]));
    }

    if (state.node_id() <= num_l1_nodes_) {
      std::reverse(state.key_buf().begin(), state.key_buf().end());
      agent.set_key(state.key_buf().data(), state.key_buf().size());
      agent.set_key(agent.query().id());
      return;
    }
    state.set_node_id(louds_.select1(state.node_id()) - state.node_id() - 1);
  }
}

bool LoudsTrie::common_prefix_search(Agent &agent) const {
  assert(agent.has_state());

  State &state = agent.state();
  if (state.status_code() == MARISA_END_OF_COMMON_PREFIX_SEARCH) {
    return false;
  }

  if (state.status_code() != MARISA_READY_TO_COMMON_PREFIX_SEARCH) {
    state.common_prefix_search_init();
    if (terminal_flags_[state.node_id()]) {
      agent.set_key(agent.query().ptr(), state.query_pos());
      agent.set_key(terminal_flags_.rank1(state.node_id()));
      return true;
    }
  }

  while (state.query_pos() < agent.query().length()) {
    if (!find_child(agent)) {
      state.set_status_code(MARISA_END_OF_COMMON_PREFIX_SEARCH);
      return false;
    }
    if (terminal_flags_[state.node_id()]) {
      agent.set_key(agent.query().ptr(), state.query_pos());
      agent.set_key(terminal_flags_.rank1(state.node_id()));
      return true;
    }
  }
  state.set_status_code(MARISA_END_OF_COMMON_PREFIX_SEARCH);
  return false;
}

bool LoudsTrie::predictive_search(Agent &agent) const {
  assert(agent.has_state());

  State &state = agent.state();
  if (state.status_code() == MARISA_END_OF_PREDICTIVE_SEARCH) {
    return false;
  }

  if (state.status_code() != MARISA_READY_TO_PREDICTIVE_SEARCH) {
    state.predictive_search_init();
    while (state.query_pos() < agent.query().length()) {
      if (!predictive_find_child(agent)) {
        state.set_status_code(MARISA_END_OF_PREDICTIVE_SEARCH);
        return false;
      }
    }

    History history;
    history.set_node_id(state.node_id());
    history.set_key_pos(state.key_buf().size());
    state.history().push_back(history);
    state.set_history_pos(1);

    if (terminal_flags_[state.node_id()]) {
      agent.set_key(state.key_buf().data(), state.key_buf().size());
      agent.set_key(terminal_flags_.rank1(state.node_id()));
      return true;
    }
  }

  for (;;) {
    if (state.history_pos() == state.history().size()) {
      const History &current = state.history().back();
      History next;
      next.set_louds_pos(louds_.select0(current.node_id()) + 1);
      next.set_node_id(next.louds_pos() - current.node_id() - 1);
      state.history().push_back(next);
    }

    History &next = state.history()[state.history_pos()];
    const bool link_flag = louds_[next.louds_pos()];
    next.set_louds_pos(next.louds_pos() + 1);
    if (link_flag) {
      state.set_history_pos(state.history_pos() + 1);
      if (link_flags_[next.node_id()]) {
        next.set_link_id(update_link_id(next.link_id(), next.node_id()));
        restore(agent, get_link(next.node_id(), next.link_id()));
      } else {
        state.key_buf().push_back(static_cast<char>(bases_[next.node_id()]));
      }
      next.set_key_pos(state.key_buf().size());

      if (terminal_flags_[next.node_id()]) {
        if (next.key_id() == MARISA_INVALID_KEY_ID) {
          next.set_key_id(terminal_flags_.rank1(next.node_id()));
        } else {
          next.set_key_id(next.key_id() + 1);
        }
        agent.set_key(state.key_buf().data(), state.key_buf().size());
        agent.set_key(next.key_id());
        return true;
      }
    } else if (state.history_pos() != 1) {
      History &current = state.history()[state.history_pos() - 1];
      current.set_node_id(current.node_id() + 1);
      const History &prev = state.history()[state.history_pos() - 2];
      state.key_buf().resize(prev.key_pos());
      state.set_history_pos(state.history_pos() - 1);
    } else {
      state.set_status_code(MARISA_END_OF_PREDICTIVE_SEARCH);
      return false;
    }
  }
}

std::size_t LoudsTrie::total_size() const {
  return louds_.total_size() + terminal_flags_.total_size() +
         link_flags_.total_size() + bases_.total_size() + extras_.total_size() +
         tail_.total_size() +
         ((next_trie_ != nullptr) ? next_trie_->total_size() : 0) +
         cache_.total_size();
}

std::size_t LoudsTrie::io_size() const {
  return Header().io_size() + louds_.io_size() + terminal_flags_.io_size() +
         link_flags_.io_size() + bases_.io_size() + extras_.io_size() +
         tail_.io_size() +
         ((next_trie_ != nullptr) ? (next_trie_->io_size() - Header().io_size())
                                  : 0) +
         cache_.io_size() + (sizeof(uint32_t) * 2);
}

void LoudsTrie::clear() noexcept {
  LoudsTrie().swap(*this);
}

void LoudsTrie::swap(LoudsTrie &rhs) noexcept {
  louds_.swap(rhs.louds_);
  terminal_flags_.swap(rhs.terminal_flags_);
  link_flags_.swap(rhs.link_flags_);
  bases_.swap(rhs.bases_);
  extras_.swap(rhs.extras_);
  tail_.swap(rhs.tail_);
  next_trie_.swap(rhs.next_trie_);
  cache_.swap(rhs.cache_);
  std::swap(cache_mask_, rhs.cache_mask_);
  std::swap(num_l1_nodes_, rhs.num_l1_nodes_);
  config_.swap(rhs.config_);
  mapper_.swap(rhs.mapper_);
}

void LoudsTrie::build_(Keyset &keyset, const Config &config) {
  Vector<Key> keys;
  keys.resize(keyset.size());
  for (std::size_t i = 0; i < keyset.size(); ++i) {
    keys[i].set_str(keyset[i].ptr(), keyset[i].length());
    keys[i].set_weight(keyset[i].weight());
  }

  Vector<uint32_t> terminals;
  build_trie(keys, &terminals, config, 1);

  using TerminalIdPair = std::pair<uint32_t, uint32_t>;
  const std::size_t pairs_size = terminals.size();
  std::unique_ptr<TerminalIdPair[]> pairs(new TerminalIdPair[pairs_size]);
  for (std::size_t i = 0; i < pairs_size; ++i) {
    pairs[i].first = terminals[i];
    pairs[i].second = static_cast<uint32_t>(i);
  }
  terminals.clear();
  std::sort(pairs.get(), pairs.get() + pairs_size);

  std::size_t node_id = 0;
  for (std::size_t i = 0; i < pairs_size; ++i) {
    while (node_id < pairs[i].first) {
      terminal_flags_.push_back(false);
      ++node_id;
    }
    if (node_id == pairs[i].first) {
      terminal_flags_.push_back(true);
      ++node_id;
    }
  }
  while (node_id < bases_.size()) {
    terminal_flags_.push_back(false);
    ++node_id;
  }
  terminal_flags_.push_back(false);
  terminal_flags_.build(false, true);

  for (std::size_t i = 0; i < keyset.size(); ++i) {
    keyset[pairs[i].second].set_id(terminal_flags_.rank1(pairs[i].first));
  }
}

template <typename T>
void LoudsTrie::build_trie(Vector<T> &keys, Vector<uint32_t> *terminals,
                           const Config &config, std::size_t trie_id) {
  build_current_trie(keys, terminals, config, trie_id);

  Vector<uint32_t> next_terminals;
  if (!keys.empty()) {
    build_next_trie(keys, &next_terminals, config, trie_id);
  }

  if (next_trie_ != nullptr) {
    config_.parse(static_cast<int>((next_trie_->num_tries() + 1)) |
                  next_trie_->tail_mode() | next_trie_->node_order());
  } else {
    config_.parse(1 | tail_.mode() | config.node_order() |
                  config.cache_level());
  }

  link_flags_.build(false, false);
  std::size_t node_id = 0;
  for (std::size_t i = 0; i < next_terminals.size(); ++i) {
    while (!link_flags_[node_id]) {
      ++node_id;
    }
    bases_[node_id] = static_cast<uint8_t>(next_terminals[i] % 256);
    next_terminals[i] /= 256;
    ++node_id;
  }
  extras_.build(next_terminals);
  fill_cache();
}

template <typename T>
void LoudsTrie::build_current_trie(Vector<T> &keys, Vector<uint32_t> *terminals,
                                   const Config &config, std::size_t trie_id) {
  for (std::size_t i = 0; i < keys.size(); ++i) {
    keys[i].set_id(i);
  }
  const std::size_t num_keys = algorithm::sort(keys.begin(), keys.end());
  reserve_cache(config, trie_id, num_keys);

  louds_.push_back(true);
  louds_.push_back(false);
  bases_.push_back('\0');
  link_flags_.push_back(false);

  Vector<T> next_keys;
  std::queue<Range> queue;
  Vector<WeightedRange> w_ranges;

  queue.push(make_range(0, keys.size(), 0));
  while (!queue.empty()) {
    const std::size_t node_id = link_flags_.size() - queue.size();

    Range range = queue.front();
    queue.pop();

    while ((range.begin() < range.end()) &&
           (keys[range.begin()].length() == range.key_pos())) {
      keys[range.begin()].set_terminal(node_id);
      range.set_begin(range.begin() + 1);
    }

    if (range.begin() == range.end()) {
      louds_.push_back(false);
      continue;
    }

    w_ranges.clear();
    double weight = double{keys[range.begin()].weight()};
    for (std::size_t i = range.begin() + 1; i < range.end(); ++i) {
      if (keys[i - 1][range.key_pos()] != keys[i][range.key_pos()]) {
        w_ranges.push_back(make_weighted_range(
            range.begin(), i, range.key_pos(), static_cast<float>(weight)));
        range.set_begin(i);
        weight = 0.0;
      }
      weight += double{keys[i].weight()};
    }
    w_ranges.push_back(make_weighted_range(range.begin(), range.end(),
                                           range.key_pos(),
                                           static_cast<float>(weight)));
    if (config.node_order() == MARISA_WEIGHT_ORDER) {
      std::stable_sort(w_ranges.begin(), w_ranges.end(),
                       std::greater<WeightedRange>());
    }

    if (node_id == 0) {
      num_l1_nodes_ = w_ranges.size();
    }

    for (std::size_t i = 0; i < w_ranges.size(); ++i) {
      WeightedRange &w_range = w_ranges[i];
      std::size_t key_pos = w_range.key_pos() + 1;
      while (key_pos < keys[w_range.begin()].length()) {
        std::size_t j;
        for (j = w_range.begin() + 1; j < w_range.end(); ++j) {
          if (keys[j - 1][key_pos] != keys[j][key_pos]) {
            break;
          }
        }
        if (j < w_range.end()) {
          break;
        }
        ++key_pos;
      }
      cache<T>(node_id, bases_.size(), w_range.weight(),
               keys[w_range.begin()][w_range.key_pos()]);

      if (key_pos == w_range.key_pos() + 1) {
        bases_.push_back(static_cast<unsigned char>(
            keys[w_range.begin()][w_range.key_pos()]));
        link_flags_.push_back(false);
      } else {
        bases_.push_back('\0');
        link_flags_.push_back(true);
        T next_key;
        next_key.set_str(keys[w_range.begin()].ptr(),
                         keys[w_range.begin()].length());
        next_key.substr(w_range.key_pos(), key_pos - w_range.key_pos());
        next_key.set_weight(w_range.weight());
        next_keys.push_back(next_key);
      }
      w_range.set_key_pos(key_pos);
      queue.push(w_range.range());
      louds_.push_back(true);
    }
    louds_.push_back(false);
  }

  louds_.push_back(false);
  louds_.build(trie_id == 1, true);
  bases_.shrink();

  build_terminals(keys, terminals);
  keys.swap(next_keys);
}

template <>
void LoudsTrie::build_next_trie(Vector<Key> &keys, Vector<uint32_t> *terminals,
                                const Config &config, std::size_t trie_id) {
  if (trie_id == config.num_tries()) {
    Vector<Entry> entries;
    entries.resize(keys.size());
    for (std::size_t i = 0; i < keys.size(); ++i) {
      entries[i].set_str(keys[i].ptr(), keys[i].length());
    }
    tail_.build(entries, terminals, config.tail_mode());
    return;
  }
  Vector<ReverseKey> reverse_keys;
  reverse_keys.resize(keys.size());
  for (std::size_t i = 0; i < keys.size(); ++i) {
    reverse_keys[i].set_str(keys[i].ptr(), keys[i].length());
    reverse_keys[i].set_weight(keys[i].weight());
  }
  keys.clear();
  next_trie_.reset(new LoudsTrie);
  next_trie_->build_trie(reverse_keys, terminals, config, trie_id + 1);
}

template <>
void LoudsTrie::build_next_trie(Vector<ReverseKey> &keys,
                                Vector<uint32_t> *terminals,
                                const Config &config, std::size_t trie_id) {
  if (trie_id == config.num_tries()) {
    Vector<Entry> entries;
    entries.resize(keys.size());
    for (std::size_t i = 0; i < keys.size(); ++i) {
      entries[i].set_str(keys[i].ptr(), keys[i].length());
    }
    tail_.build(entries, terminals, config.tail_mode());
    return;
  }
  next_trie_.reset(new LoudsTrie);
  next_trie_->build_trie(keys, terminals, config, trie_id + 1);
}

template <typename T>
void LoudsTrie::build_terminals(const Vector<T> &keys,
                                Vector<uint32_t> *terminals) const {
  Vector<uint32_t> temp;
  temp.resize(keys.size());
  for (std::size_t i = 0; i < keys.size(); ++i) {
    temp[keys[i].id()] = static_cast<uint32_t>(keys[i].terminal());
  }
  terminals->swap(temp);
}

template <>
void LoudsTrie::cache<Key>(std::size_t parent, std::size_t child, float weight,
                           char label) {
  assert(parent < child);

  const std::size_t cache_id = get_cache_id(parent, label);
  if (weight > cache_[cache_id].weight()) {
    cache_[cache_id].set_parent(parent);
    cache_[cache_id].set_child(child);
    cache_[cache_id].set_weight(weight);
  }
}

void LoudsTrie::reserve_cache(const Config &config, std::size_t trie_id,
                              std::size_t num_keys) {
  std::size_t cache_size = (trie_id == 1) ? 256 : 1;
  while (cache_size < (num_keys / config.cache_level())) {
    cache_size *= 2;
  }
  cache_.resize(cache_size);
  cache_mask_ = cache_size - 1;
}

template <>
void LoudsTrie::cache<ReverseKey>(std::size_t parent, std::size_t child,
                                  float weight, char) {
  assert(parent < child);

  const std::size_t cache_id = get_cache_id(child);
  if (weight > cache_[cache_id].weight()) {
    cache_[cache_id].set_parent(parent);
    cache_[cache_id].set_child(child);
    cache_[cache_id].set_weight(weight);
  }
}

void LoudsTrie::fill_cache() {
  for (std::size_t i = 0; i < cache_.size(); ++i) {
    const std::size_t node_id = cache_[i].child();
    if (node_id != 0) {
      cache_[i].set_base(bases_[node_id]);
      cache_[i].set_extra(!link_flags_[node_id]
                              ? MARISA_INVALID_EXTRA
                              : extras_[link_flags_.rank1(node_id)]);
    } else {
      cache_[i].set_parent(UINT32_MAX);
      cache_[i].set_child(UINT32_MAX);
    }
  }
}

void LoudsTrie::map_(Mapper &mapper) {
  louds_.map(mapper);
  terminal_flags_.map(mapper);
  link_flags_.map(mapper);
  bases_.map(mapper);
  extras_.map(mapper);
  tail_.map(mapper);
  if ((link_flags_.num_1s() != 0) && tail_.empty()) {
    next_trie_.reset(new LoudsTrie);
    next_trie_->map_(mapper);
  }
  cache_.map(mapper);
  cache_mask_ = cache_.size() - 1;
  {
    uint32_t temp_num_l1_nodes;
    mapper.map(&temp_num_l1_nodes);
    num_l1_nodes_ = temp_num_l1_nodes;
  }
  {
    uint32_t temp_config_flags;
    mapper.map(&temp_config_flags);
    config_.parse(static_cast<int>(temp_config_flags));
  }
}

void LoudsTrie::read_(Reader &reader) {
  louds_.read(reader);
  terminal_flags_.read(reader);
  link_flags_.read(reader);
  bases_.read(reader);
  extras_.read(reader);
  tail_.read(reader);
  if ((link_flags_.num_1s() != 0) && tail_.empty()) {
    next_trie_.reset(new LoudsTrie);
    next_trie_->read_(reader);
  }
  cache_.read(reader);
  cache_mask_ = cache_.size() - 1;
  {
    uint32_t temp_num_l1_nodes;
    reader.read(&temp_num_l1_nodes);
    num_l1_nodes_ = temp_num_l1_nodes;
  }
  {
    uint32_t temp_config_flags;
    reader.read(&temp_config_flags);
    config_.parse(static_cast<int>(temp_config_flags));
  }
}

void LoudsTrie::write_(Writer &writer) const {
  louds_.write(writer);
  terminal_flags_.write(writer);
  link_flags_.write(writer);
  bases_.write(writer);
  extras_.write(writer);
  tail_.write(writer);
  if (next_trie_ != nullptr) {
    next_trie_->write_(writer);
  }
  cache_.write(writer);
  writer.write(static_cast<uint32_t>(num_l1_nodes_));
  writer.write(static_cast<uint32_t>(config_.flags()));
}

bool LoudsTrie::find_child(Agent &agent) const {
  assert(agent.state().query_pos() < agent.query().length());

  State &state = agent.state();
  const std::size_t cache_id =
      get_cache_id(state.node_id(), agent.query()[state.query_pos()]);
  if (state.node_id() == cache_[cache_id].parent()) {
    if (cache_[cache_id].extra() != MARISA_INVALID_EXTRA) {
      if (!match(agent, cache_[cache_id].link())) {
        return false;
      }
    } else {
      state.set_query_pos(state.query_pos() + 1);
    }
    state.set_node_id(cache_[cache_id].child());
    return true;
  }

  std::size_t louds_pos = louds_.select0(state.node_id()) + 1;
  if (!louds_[louds_pos]) {
    return false;
  }
  state.set_node_id(louds_pos - state.node_id() - 1);
  std::size_t link_id = MARISA_INVALID_LINK_ID;
  do {
    if (link_flags_[state.node_id()]) {
      link_id = update_link_id(link_id, state.node_id());
      const std::size_t prev_query_pos = state.query_pos();
      if (match(agent, get_link(state.node_id(), link_id))) {
        return true;
      }
      if (state.query_pos() != prev_query_pos) {
        return false;
      }
    } else if (bases_[state.node_id()] ==
               static_cast<uint8_t>(agent.query()[state.query_pos()])) {
      state.set_query_pos(state.query_pos() + 1);
      return true;
    }
    state.set_node_id(state.node_id() + 1);
    ++louds_pos;
  } while (louds_[louds_pos]);
  return false;
}

bool LoudsTrie::predictive_find_child(Agent &agent) const {
  assert(agent.state().query_pos() < agent.query().length());

  State &state = agent.state();
  const std::size_t cache_id =
      get_cache_id(state.node_id(), agent.query()[state.query_pos()]);
  if (state.node_id() == cache_[cache_id].parent()) {
    if (cache_[cache_id].extra() != MARISA_INVALID_EXTRA) {
      if (!prefix_match(agent, cache_[cache_id].link())) {
        return false;
      }
    } else {
      state.key_buf().push_back(cache_[cache_id].label());
      state.set_query_pos(state.query_pos() + 1);
    }
    state.set_node_id(cache_[cache_id].child());
    return true;
  }

  std::size_t louds_pos = louds_.select0(state.node_id()) + 1;
  if (!louds_[louds_pos]) {
    return false;
  }
  state.set_node_id(louds_pos - state.node_id() - 1);
  std::size_t link_id = MARISA_INVALID_LINK_ID;
  do {
    if (link_flags_[state.node_id()]) {
      link_id = update_link_id(link_id, state.node_id());
      const std::size_t prev_query_pos = state.query_pos();
      if (prefix_match(agent, get_link(state.node_id(), link_id))) {
        return true;
      }
      if (state.query_pos() != prev_query_pos) {
        return false;
      }
    } else if (bases_[state.node_id()] ==
               static_cast<uint8_t>(agent.query()[state.query_pos()])) {
      state.key_buf().push_back(static_cast<char>(bases_[state.node_id()]));
      state.set_query_pos(state.query_pos() + 1);
      return true;
    }
    state.set_node_id(state.node_id() + 1);
    ++louds_pos;
  } while (louds_[louds_pos]);
  return false;
}

void LoudsTrie::restore(Agent &agent, std::size_t link) const {
  if (next_trie_ != nullptr) {
    next_trie_->restore_(agent, link);
  } else {
    tail_.restore(agent, link);
  }
}

bool LoudsTrie::match(Agent &agent, std::size_t link) const {
  if (next_trie_ != nullptr) {
    return next_trie_->match_(agent, link);
  }
  return tail_.match(agent, link);
}

bool LoudsTrie::prefix_match(Agent &agent, std::size_t link) const {
  if (next_trie_ != nullptr) {
    return next_trie_->prefix_match_(agent, link);
  }
  return tail_.prefix_match(agent, link);
}

void LoudsTrie::restore_(Agent &agent, std::size_t node_id) const {
  assert(node_id != 0);

  State &state = agent.state();
  for (;;) {
    const std::size_t cache_id = get_cache_id(node_id);
    if (node_id == cache_[cache_id].child()) {
      if (cache_[cache_id].extra() != MARISA_INVALID_EXTRA) {
        restore(agent, cache_[cache_id].link());
      } else {
        state.key_buf().push_back(cache_[cache_id].label());
      }

      node_id = cache_[cache_id].parent();
      if (node_id == 0) {
        return;
      }
      continue;
    }

    if (link_flags_[node_id]) {
      restore(agent, get_link(node_id));
    } else {
      state.key_buf().push_back(static_cast<char>(bases_[node_id]));
    }

    if (node_id <= num_l1_nodes_) {
      return;
    }
    node_id = louds_.select1(node_id) - node_id - 1;
  }
}

bool LoudsTrie::match_(Agent &agent, std::size_t node_id) const {
  assert(agent.state().query_pos() < agent.query().length());
  assert(node_id != 0);

  State &state = agent.state();
  for (;;) {
    const std::size_t cache_id = get_cache_id(node_id);
    if (node_id == cache_[cache_id].child()) {
      if (cache_[cache_id].extra() != MARISA_INVALID_EXTRA) {
        if (!match(agent, cache_[cache_id].link())) {
          return false;
        }
      } else if (cache_[cache_id].label() == agent.query()[state.query_pos()]) {
        state.set_query_pos(state.query_pos() + 1);
      } else {
        return false;
      }

      node_id = cache_[cache_id].parent();
      if (node_id == 0) {
        return true;
      }
      if (state.query_pos() >= agent.query().length()) {
        return false;
      }
      continue;
    }

    if (link_flags_[node_id]) {
      if (next_trie_ != nullptr) {
        if (!match(agent, get_link(node_id))) {
          return false;
        }
      } else if (!tail_.match(agent, get_link(node_id))) {
        return false;
      }
    } else if (bases_[node_id] ==
               static_cast<uint8_t>(agent.query()[state.query_pos()])) {
      state.set_query_pos(state.query_pos() + 1);
    } else {
      return false;
    }

    if (node_id <= num_l1_nodes_) {
      return true;
    }
    if (state.query_pos() >= agent.query().length()) {
      return false;
    }
    node_id = louds_.select1(node_id) - node_id - 1;
  }
}

bool LoudsTrie::prefix_match_(Agent &agent, std::size_t node_id) const {
  assert(agent.state().query_pos() < agent.query().length());
  assert(node_id != 0);

  State &state = agent.state();
  for (;;) {
    const std::size_t cache_id = get_cache_id(node_id);
    if (node_id == cache_[cache_id].child()) {
      if (cache_[cache_id].extra() != MARISA_INVALID_EXTRA) {
        if (!prefix_match(agent, cache_[cache_id].link())) {
          return false;
        }
      } else if (cache_[cache_id].label() == agent.query()[state.query_pos()]) {
        state.key_buf().push_back(cache_[cache_id].label());
        state.set_query_pos(state.query_pos() + 1);
      } else {
        return false;
      }

      node_id = cache_[cache_id].parent();
      if (node_id == 0) {
        return true;
      }
    } else {
      if (link_flags_[node_id]) {
        if (!prefix_match(agent, get_link(node_id))) {
          return false;
        }
      } else if (bases_[node_id] ==
                 static_cast<uint8_t>(agent.query()[state.query_pos()])) {
        state.key_buf().push_back(static_cast<char>(bases_[node_id]));
        state.set_query_pos(state.query_pos() + 1);
      } else {
        return false;
      }

      if (node_id <= num_l1_nodes_) {
        return true;
      }
      node_id = louds_.select1(node_id) - node_id - 1;
    }

    if (state.query_pos() >= agent.query().length()) {
      restore_(agent, node_id);
      return true;
    }
  }
}

std::size_t LoudsTrie::get_cache_id(std::size_t node_id, char label) const {
  return (node_id ^ (node_id << 5) ^ static_cast<uint8_t>(label)) & cache_mask_;
}

std::size_t LoudsTrie::get_cache_id(std::size_t node_id) const {
  return node_id & cache_mask_;
}

std::size_t LoudsTrie::get_link(std::size_t node_id) const {
  return bases_[node_id] | (extras_[link_flags_.rank1(node_id)] * 256);
}

std::size_t LoudsTrie::get_link(std::size_t node_id,
                                std::size_t link_id) const {
  return bases_[node_id] | (extras_[link_id] * 256);
}

std::size_t LoudsTrie::update_link_id(std::size_t link_id,
                                      std::size_t node_id) const {
  return (link_id == MARISA_INVALID_LINK_ID) ? link_flags_.rank1(node_id)
                                             : (link_id + 1);
}

}  // namespace marisa::grimoire::trie
