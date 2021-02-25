#include "marisa/grimoire/algorithm.h"
#include "marisa/grimoire/trie/state.h"
#include "marisa/grimoire/trie/tail.h"

namespace marisa {
namespace grimoire {
namespace trie {

Tail::Tail() : buf_(), end_flags_() {}

void Tail::build(Vector<Entry> &entries, Vector<UInt32> *offsets,
    TailMode mode) {
  MARISA_THROW_IF(offsets == NULL, MARISA_NULL_ERROR);

  switch (mode) {
    case MARISA_TEXT_TAIL: {
      for (std::size_t i = 0; i < entries.size(); ++i) {
        const char * const ptr = entries[i].ptr();
        const std::size_t length = entries[i].length();
        for (std::size_t j = 0; j < length; ++j) {
          if (ptr[j] == '\0') {
            mode = MARISA_BINARY_TAIL;
            break;
          }
        }
        if (mode == MARISA_BINARY_TAIL) {
          break;
        }
      }
      break;
    }
    case MARISA_BINARY_TAIL: {
      break;
    }
    default: {
      MARISA_THROW(MARISA_CODE_ERROR, "undefined tail mode");
    }
  }

  Tail temp;
  temp.build_(entries, offsets, mode);
  swap(temp);
}

void Tail::map(Mapper &mapper) {
  Tail temp;
  temp.map_(mapper);
  swap(temp);
}

void Tail::read(Reader &reader) {
  Tail temp;
  temp.read_(reader);
  swap(temp);
}

void Tail::write(Writer &writer) const {
  write_(writer);
}

void Tail::restore(Agent &agent, std::size_t offset) const {
  MARISA_DEBUG_IF(buf_.empty(), MARISA_STATE_ERROR);

  State &state = agent.state();
  if (end_flags_.empty()) {
    for (const char *ptr = &buf_[offset]; *ptr != '\0'; ++ptr) {
      state.key_buf().push_back(*ptr);
    }
  } else {
    do {
      state.key_buf().push_back(buf_[offset]);
    } while (!end_flags_[offset++]);
  }
}

bool Tail::match(Agent &agent, std::size_t offset) const {
  MARISA_DEBUG_IF(buf_.empty(), MARISA_STATE_ERROR);
  MARISA_DEBUG_IF(agent.state().query_pos() >= agent.query().length(),
      MARISA_BOUND_ERROR);

  State &state = agent.state();
  if (end_flags_.empty()) {
    const char * const ptr = &buf_[offset] - state.query_pos();
    do {
      if (ptr[state.query_pos()] != agent.query()[state.query_pos()]) {
        return false;
      }
      state.set_query_pos(state.query_pos() + 1);
      if (ptr[state.query_pos()] == '\0') {
        return true;
      }
    } while (state.query_pos() < agent.query().length());
    return false;
  } else {
    do {
      if (buf_[offset] != agent.query()[state.query_pos()]) {
        return false;
      }
      state.set_query_pos(state.query_pos() + 1);
      if (end_flags_[offset++]) {
        return true;
      }
    } while (state.query_pos() < agent.query().length());
    return false;
  }
}

bool Tail::prefix_match(Agent &agent, std::size_t offset) const {
  MARISA_DEBUG_IF(buf_.empty(), MARISA_STATE_ERROR);

  State &state = agent.state();
  if (end_flags_.empty()) {
    const char *ptr = &buf_[offset] - state.query_pos();
    do {
      if (ptr[state.query_pos()] != agent.query()[state.query_pos()]) {
        return false;
      }
      state.key_buf().push_back(ptr[state.query_pos()]);
      state.set_query_pos(state.query_pos() + 1);
      if (ptr[state.query_pos()] == '\0') {
        return true;
      }
    } while (state.query_pos() < agent.query().length());
    ptr += state.query_pos();
    do {
      state.key_buf().push_back(*ptr);
    } while (*++ptr != '\0');
    return true;
  } else {
    do {
      if (buf_[offset] != agent.query()[state.query_pos()]) {
        return false;
      }
      state.key_buf().push_back(buf_[offset]);
      state.set_query_pos(state.query_pos() + 1);
      if (end_flags_[offset++]) {
        return true;
      }
    } while (state.query_pos() < agent.query().length());
    do {
      state.key_buf().push_back(buf_[offset]);
    } while (!end_flags_[offset++]);
    return true;
  }
}

void Tail::clear() {
  Tail().swap(*this);
}

void Tail::swap(Tail &rhs) {
  buf_.swap(rhs.buf_);
  end_flags_.swap(rhs.end_flags_);
}

void Tail::build_(Vector<Entry> &entries, Vector<UInt32> *offsets,
    TailMode mode) {
  for (std::size_t i = 0; i < entries.size(); ++i) {
    entries[i].set_id(i);
  }
  Algorithm().sort(entries.begin(), entries.end());

  Vector<UInt32> temp_offsets;
  temp_offsets.resize(entries.size(), 0);

  const Entry dummy;
  const Entry *last = &dummy;
  for (std::size_t i = entries.size(); i > 0; --i) {
    const Entry &current = entries[i - 1];
    MARISA_THROW_IF(current.length() == 0, MARISA_RANGE_ERROR);
    std::size_t match = 0;
    while ((match < current.length()) && (match < last->length()) &&
        ((*last)[match] == current[match])) {
      ++match;
    }
    if ((match == current.length()) && (last->length() != 0)) {
      temp_offsets[current.id()] = (UInt32)(
          temp_offsets[last->id()] + (last->length() - match));
    } else {
      temp_offsets[current.id()] = (UInt32)buf_.size();
      for (std::size_t j = 1; j <= current.length(); ++j) {
        buf_.push_back(current[current.length() - j]);
      }
      if (mode == MARISA_TEXT_TAIL) {
        buf_.push_back('\0');
      } else {
        for (std::size_t j = 1; j < current.length(); ++j) {
          end_flags_.push_back(false);
        }
        end_flags_.push_back(true);
      }
      MARISA_THROW_IF(buf_.size() > MARISA_UINT32_MAX, MARISA_SIZE_ERROR);
    }
    last = &current;
  }
  buf_.shrink();

  offsets->swap(temp_offsets);
}

void Tail::map_(Mapper &mapper) {
  buf_.map(mapper);
  end_flags_.map(mapper);
}

void Tail::read_(Reader &reader) {
  buf_.read(reader);
  end_flags_.read(reader);
}

void Tail::write_(Writer &writer) const {
  buf_.write(writer);
  end_flags_.write(writer);
}

}  // namespace trie
}  // namespace grimoire
}  // namespace marisa
