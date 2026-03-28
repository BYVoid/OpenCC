#include <marisa/grimoire/trie/config.h>
#include <marisa/grimoire/trie/header.h>
#include <marisa/grimoire/trie/key.h>
#include <marisa/grimoire/trie/range.h>
#include <marisa/grimoire/trie/state.h>
#include <marisa/grimoire/trie/tail.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <exception>
#include <sstream>

#include "marisa-assert.h"

namespace {

void TestConfig() {
  TEST_START();

  marisa::grimoire::trie::Config config;

  ASSERT(config.num_tries() == MARISA_DEFAULT_NUM_TRIES);
  ASSERT(config.tail_mode() == MARISA_DEFAULT_TAIL);
  ASSERT(config.node_order() == MARISA_DEFAULT_ORDER);
  ASSERT(config.cache_level() == MARISA_DEFAULT_CACHE);

  config.parse(10 | MARISA_BINARY_TAIL | MARISA_LABEL_ORDER |
               MARISA_TINY_CACHE);

  ASSERT(config.num_tries() == 10);
  ASSERT(config.tail_mode() == MARISA_BINARY_TAIL);
  ASSERT(config.node_order() == MARISA_LABEL_ORDER);
  ASSERT(config.cache_level() == MARISA_TINY_CACHE);

  config.parse(0);

  ASSERT(config.num_tries() == MARISA_DEFAULT_NUM_TRIES);
  ASSERT(config.tail_mode() == MARISA_DEFAULT_TAIL);
  ASSERT(config.node_order() == MARISA_DEFAULT_ORDER);
  ASSERT(config.cache_level() == MARISA_DEFAULT_CACHE);

  TEST_END();
}

void TestHeader() {
  TEST_START();

  marisa::grimoire::trie::Header header;

  {
    marisa::grimoire::Writer writer;
    writer.open("trie-test.dat");
    header.write(writer);
  }

  {
    marisa::grimoire::Mapper mapper;
    mapper.open("trie-test.dat");
    header.map(mapper);
  }

  {
    marisa::grimoire::Reader reader;
    reader.open("trie-test.dat");
    header.read(reader);
  }

  TEST_END();
}

void TestKey() {
  TEST_START();

  marisa::grimoire::trie::Key key;

  ASSERT(key.ptr() == nullptr);
  ASSERT(key.length() == 0);
  ASSERT(key.id() == 0);
  ASSERT(key.terminal() == 0);

  const char *str = "xyz";

  key.set_str(str, 3);
  key.set_weight(10.0F);
  key.set_id(20);

  ASSERT(key.ptr() == str);
  ASSERT(key.length() == 3);
  ASSERT(key[0] == 'x');
  ASSERT(key[1] == 'y');
  ASSERT(key[2] == 'z');
  ASSERT(key.weight() == 10.0F);
  ASSERT(key.id() == 20);

  key.set_terminal(30);
  ASSERT(key.terminal() == 30);

  key.substr(1, 2);

  ASSERT(key.ptr() == str + 1);
  ASSERT(key.length() == 2);
  ASSERT(key[0] == 'y');
  ASSERT(key[1] == 'z');

  marisa::grimoire::trie::Key key2;
  key2.set_str("abc", 3);

  ASSERT(key == key);
  ASSERT(key != key2);
  ASSERT(key > key2);
  ASSERT(key2 < key);

  marisa::grimoire::trie::ReverseKey r_key;

  ASSERT(r_key.ptr() == nullptr);
  ASSERT(r_key.length() == 0);
  ASSERT(r_key.id() == 0);
  ASSERT(r_key.terminal() == 0);

  r_key.set_str(str, 3);
  r_key.set_weight(100.0F);
  r_key.set_id(200);

  ASSERT(r_key.ptr() == str);
  ASSERT(r_key.length() == 3);
  ASSERT(r_key[0] == 'z');
  ASSERT(r_key[1] == 'y');
  ASSERT(r_key[2] == 'x');
  ASSERT(r_key.weight() == 100.0F);
  ASSERT(r_key.id() == 200);

  r_key.set_terminal(300);
  ASSERT(r_key.terminal() == 300);

  r_key.substr(1, 2);

  ASSERT(r_key.ptr() == str);
  ASSERT(r_key.length() == 2);
  ASSERT(r_key[0] == 'y');
  ASSERT(r_key[1] == 'x');

  marisa::grimoire::trie::ReverseKey r_key2;
  r_key2.set_str("abc", 3);

  ASSERT(r_key == r_key);
  ASSERT(r_key != r_key2);
  ASSERT(r_key > r_key2);
  ASSERT(r_key2 < r_key);

  TEST_END();
}

void TestRange() {
  TEST_START();

  marisa::grimoire::trie::Range range;

  ASSERT(range.begin() == 0);
  ASSERT(range.end() == 0);
  ASSERT(range.key_pos() == 0);

  range.set_begin(1);
  range.set_end(2);
  range.set_key_pos(3);

  ASSERT(range.begin() == 1);
  ASSERT(range.end() == 2);
  ASSERT(range.key_pos() == 3);

  range = marisa::grimoire::trie::make_range(10, 20, 30);

  ASSERT(range.begin() == 10);
  ASSERT(range.end() == 20);
  ASSERT(range.key_pos() == 30);

  marisa::grimoire::trie::WeightedRange w_range;

  ASSERT(w_range.begin() == 0);
  ASSERT(w_range.end() == 0);
  ASSERT(w_range.key_pos() == 0);
  ASSERT(w_range.weight() == 0.0F);

  w_range.set_begin(10);
  w_range.set_end(20);
  w_range.set_key_pos(30);
  w_range.set_weight(40.0F);

  ASSERT(w_range.begin() == 10);
  ASSERT(w_range.end() == 20);
  ASSERT(w_range.key_pos() == 30);
  ASSERT(w_range.weight() == 40.0F);

  marisa::grimoire::trie::WeightedRange w_range2 =
      marisa::grimoire::trie::make_weighted_range(100, 200, 300, 400.0F);

  ASSERT(w_range2.begin() == 100);
  ASSERT(w_range2.end() == 200);
  ASSERT(w_range2.key_pos() == 300);
  ASSERT(w_range2.weight() == 400.0F);

  ASSERT(w_range < w_range2);
  ASSERT(w_range2 > w_range);

  TEST_END();
}

void TestEntry() {
  TEST_START();

  marisa::grimoire::trie::Entry entry;

  ASSERT(entry.length() == 0);
  ASSERT(entry.id() == 0);

  const char *str = "XYZ";

  entry.set_str(str, 3);
  entry.set_id(123);

  ASSERT(entry.ptr() == str);
  ASSERT(entry.length() == 3);
  ASSERT(entry[0] == 'Z');
  ASSERT(entry[1] == 'Y');
  ASSERT(entry[2] == 'X');
  ASSERT(entry.id() == 123);

  TEST_END();
}

void TestTextTail() {
  TEST_START();

  marisa::grimoire::trie::Tail tail;
  marisa::grimoire::Vector<marisa::grimoire::trie::Entry> entries;
  marisa::grimoire::Vector<std::uint32_t> offsets;
  tail.build(entries, &offsets, MARISA_TEXT_TAIL);

  ASSERT(tail.mode() == MARISA_TEXT_TAIL);
  ASSERT(tail.size() == 0);
  ASSERT(tail.empty());
  ASSERT(tail.total_size() == tail.size());
  ASSERT(tail.io_size() == (sizeof(std::uint64_t) * 6));

  ASSERT(offsets.empty());

  marisa::grimoire::trie::Entry entry;
  entry.set_str("X", 1);
  entries.push_back(entry);

  tail.build(entries, &offsets, MARISA_TEXT_TAIL);

  ASSERT(tail.mode() == MARISA_TEXT_TAIL);
  ASSERT(tail.size() == 2);
  ASSERT(!tail.empty());
  ASSERT(tail.total_size() == tail.size());
  ASSERT(tail.io_size() == (sizeof(std::uint64_t) * 7));

  ASSERT(offsets.size() == entries.size());
  ASSERT(offsets[0] == 0);
  ASSERT(tail[offsets[0]] == 'X');
  ASSERT(tail[offsets[0] + 1] == '\0');

  entries.clear();
  entry.set_str("abc", 3);
  entries.push_back(entry);
  entry.set_str("bc", 2);
  entries.push_back(entry);
  entry.set_str("abc", 3);
  entries.push_back(entry);
  entry.set_str("c", 1);
  entries.push_back(entry);
  entry.set_str("ABC", 3);
  entries.push_back(entry);
  entry.set_str("AB", 2);
  entries.push_back(entry);

  tail.build(entries, &offsets, MARISA_TEXT_TAIL);
  std::sort(entries.begin(), entries.end(),
            marisa::grimoire::trie::Entry::IDComparer());

  ASSERT(tail.size() == 11);
  ASSERT(offsets.size() == entries.size());
  for (std::size_t i = 0; i < entries.size(); ++i) {
    const char *const ptr = &tail[offsets[i]];
    ASSERT(std::strlen(ptr) == entries[i].length());
    ASSERT(std::strcmp(ptr, entries[i].ptr()) == 0);
  }

  {
    marisa::grimoire::Writer writer;
    writer.open("trie-test.dat");
    tail.write(writer);
  }

  tail.clear();

  ASSERT(tail.size() == 0);
  ASSERT(tail.total_size() == tail.size());

  {
    marisa::grimoire::Mapper mapper;
    mapper.open("trie-test.dat");
    tail.map(mapper);

    ASSERT(tail.mode() == MARISA_TEXT_TAIL);
    ASSERT(tail.size() == 11);
    for (std::size_t i = 0; i < entries.size(); ++i) {
      const char *const ptr = &tail[offsets[i]];
      ASSERT(std::strlen(ptr) == entries[i].length());
      ASSERT(std::strcmp(ptr, entries[i].ptr()) == 0);
    }
    tail.clear();
  }

  {
    marisa::grimoire::Reader reader;
    reader.open("trie-test.dat");
    tail.read(reader);
  }

  ASSERT(tail.size() == 11);
  ASSERT(offsets.size() == entries.size());
  for (std::size_t i = 0; i < entries.size(); ++i) {
    const char *const ptr = &tail[offsets[i]];
    ASSERT(std::strlen(ptr) == entries[i].length());
    ASSERT(std::strcmp(ptr, entries[i].ptr()) == 0);
  }

  {
    std::stringstream stream;
    marisa::grimoire::Writer writer;
    writer.open(stream);
    tail.write(writer);
    tail.clear();
    marisa::grimoire::Reader reader;
    reader.open(stream);
    tail.read(reader);
  }

  ASSERT(tail.size() == 11);
  ASSERT(offsets.size() == entries.size());
  for (std::size_t i = 0; i < entries.size(); ++i) {
    const char *const ptr = &tail[offsets[i]];
    ASSERT(std::strlen(ptr) == entries[i].length());
    ASSERT(std::strcmp(ptr, entries[i].ptr()) == 0);
  }

  TEST_END();
}

void TestBinaryTail() {
  TEST_START();

  marisa::grimoire::trie::Tail tail;
  marisa::grimoire::Vector<marisa::grimoire::trie::Entry> entries;
  marisa::grimoire::Vector<std::uint32_t> offsets;
  tail.build(entries, &offsets, MARISA_BINARY_TAIL);

  ASSERT(tail.mode() == MARISA_TEXT_TAIL);
  ASSERT(tail.size() == 0);
  ASSERT(tail.empty());
  ASSERT(tail.total_size() == tail.size());
  ASSERT(tail.io_size() == (sizeof(std::uint64_t) * 6));

  ASSERT(offsets.empty());

  marisa::grimoire::trie::Entry entry;
  entry.set_str("X", 1);
  entries.push_back(entry);

  tail.build(entries, &offsets, MARISA_BINARY_TAIL);

  ASSERT(tail.mode() == MARISA_BINARY_TAIL);
  ASSERT(tail.size() == 1);
  ASSERT(!tail.empty());
  ASSERT(tail.total_size() == (tail.size() + sizeof(std::uint64_t)));
  ASSERT(tail.io_size() == (sizeof(std::uint64_t) * 8));

  ASSERT(offsets.size() == entries.size());
  ASSERT(offsets[0] == 0);

  const char binary_entry[] = {'N', 'P', '\0', 'T', 'r', 'i', 'e'};
  entries[0].set_str(binary_entry, sizeof(binary_entry));

  tail.build(entries, &offsets, MARISA_TEXT_TAIL);

  ASSERT(tail.mode() == MARISA_BINARY_TAIL);
  ASSERT(tail.size() == entries[0].length());

  ASSERT(offsets.size() == entries.size());
  ASSERT(offsets[0] == 0);

  entries.clear();
  entry.set_str("abc", 3);
  entries.push_back(entry);
  entry.set_str("bc", 2);
  entries.push_back(entry);
  entry.set_str("abc", 3);
  entries.push_back(entry);
  entry.set_str("c", 1);
  entries.push_back(entry);
  entry.set_str("ABC", 3);
  entries.push_back(entry);
  entry.set_str("AB", 2);
  entries.push_back(entry);

  tail.build(entries, &offsets, MARISA_BINARY_TAIL);
  std::sort(entries.begin(), entries.end(),
            marisa::grimoire::trie::Entry::IDComparer());

  ASSERT(tail.mode() == MARISA_BINARY_TAIL);
  ASSERT(tail.size() == 8);
  ASSERT(offsets.size() == entries.size());
  for (std::size_t i = 0; i < entries.size(); ++i) {
    const char *const ptr = &tail[offsets[i]];
    ASSERT(std::memcmp(ptr, entries[i].ptr(), entries[i].length()) == 0);
  }

  TEST_END();
}

void TestHistory() {
  TEST_START();

  marisa::grimoire::trie::History history;

  ASSERT(history.node_id() == 0);
  ASSERT(history.louds_pos() == 0);
  ASSERT(history.key_pos() == 0);
  ASSERT(history.link_id() == MARISA_INVALID_LINK_ID);
  ASSERT(history.key_id() == MARISA_INVALID_KEY_ID);

  history.set_node_id(100);
  history.set_louds_pos(200);
  history.set_key_pos(300);
  history.set_link_id(400);
  history.set_key_id(500);

  ASSERT(history.node_id() == 100);
  ASSERT(history.louds_pos() == 200);
  ASSERT(history.key_pos() == 300);
  ASSERT(history.link_id() == 400);
  ASSERT(history.key_id() == 500);

  TEST_END();
}

void TestState() {
  TEST_START();

  marisa::grimoire::trie::State state;

  ASSERT(state.key_buf().empty());
  ASSERT(state.history().empty());
  ASSERT(state.node_id() == 0);
  ASSERT(state.query_pos() == 0);
  ASSERT(state.history_pos() == 0);
  ASSERT(state.status_code() == marisa::grimoire::trie::MARISA_READY_TO_ALL);

  state.set_node_id(10);
  state.set_query_pos(100);
  state.set_history_pos(1000);
  state.set_status_code(
      marisa::grimoire::trie::MARISA_END_OF_PREDICTIVE_SEARCH);

  ASSERT(state.node_id() == 10);
  ASSERT(state.query_pos() == 100);
  ASSERT(state.history_pos() == 1000);
  ASSERT(state.status_code() ==
         marisa::grimoire::trie::MARISA_END_OF_PREDICTIVE_SEARCH);

  state.lookup_init();
  ASSERT(state.status_code() == marisa::grimoire::trie::MARISA_READY_TO_ALL);

  state.reverse_lookup_init();
  ASSERT(state.status_code() == marisa::grimoire::trie::MARISA_READY_TO_ALL);

  state.common_prefix_search_init();
  ASSERT(state.status_code() ==
         marisa::grimoire::trie::MARISA_READY_TO_COMMON_PREFIX_SEARCH);

  state.predictive_search_init();
  ASSERT(state.status_code() ==
         marisa::grimoire::trie::MARISA_READY_TO_PREDICTIVE_SEARCH);

  TEST_END();
}

}  // namespace

int main() try {
  TestConfig();
  TestHeader();
  TestKey();
  TestRange();
  TestEntry();
  TestTextTail();
  TestBinaryTail();
  TestHistory();
  TestState();

  return 0;
} catch (const std::exception &ex) {
  std::cerr << ex.what() << "\n";
  throw;
}
