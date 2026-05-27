#include <marisa.h>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "marisa-assert.h"

namespace {

std::random_device seed_gen;
std::mt19937 random_engine(seed_gen());

void TestSwap() {
  TEST_START();

  int x = 100, y = 200;
  std::swap(x, y);
  ASSERT(x == 200);
  ASSERT(y == 100);

  double a = 1.23, b = 2.34;
  std::swap(a, b);
  ASSERT(a == 2.34);
  ASSERT(b == 1.23);

  TEST_END();
}

void TestException() {
  TEST_START();

  try {
    MARISA_THROW(std::runtime_error, "Message");
  } catch (const std::exception &ex) {
    std::stringstream s;
    s << __FILE__ << ":" << (__LINE__ - 3) << ": std::runtime_error: Message";
    ASSERT(ex.what() == s.str());
  }

  EXCEPT(MARISA_THROW(std::runtime_error, "OK"), std::runtime_error);
  EXCEPT(MARISA_THROW(std::invalid_argument, "NULL"), std::invalid_argument);

  TEST_END();
}

void TestKey() {
  TEST_START();

  const char *const str = "apple";

  marisa::Key key;

  ASSERT(key.ptr() == nullptr);
  ASSERT(key.length() == 0);

  key.set_str(str);

  ASSERT(key.ptr() == str);
  ASSERT(key.length() == std::strlen(str));

  key.set_str(str, 4);

  ASSERT(key.ptr() == str);
  ASSERT(key.length() == 4);

  const std::string_view view = "orange";
  key.set_str(view);

  ASSERT(key.ptr() == view.data());
  ASSERT(key.length() == 6);

  // Compare string_view, emphasizing that it is not a pointer comparison.
  ASSERT(key.str() == std::string("orange"));

  key.set_weight(1.0);

  ASSERT(key.weight() == 1.0F);

  key.set_id(100);

  ASSERT(key.id() == 100);

  TEST_END();
}

void TestKeyset() {
  TEST_START();

  marisa::Keyset keyset;

  ASSERT(keyset.size() == 0);
  ASSERT(keyset.empty());
  ASSERT(keyset.total_length() == 0);

  std::vector<std::string> keys;
  keys.push_back("apple");
  keys.push_back("orange");
  keys.push_back("banana");

  std::size_t total_length = 0;
  for (std::size_t i = 0; i < keys.size(); ++i) {
    keyset.push_back(keys[i].c_str());
    ASSERT(keyset.size() == (i + 1));
    ASSERT(!keyset.empty());

    total_length += keys[i].length();
    ASSERT(keyset.total_length() == total_length);

    ASSERT(keyset[i].length() == keys[i].length());
    ASSERT(std::memcmp(keyset[i].ptr(), keys[i].c_str(), keyset[i].length()) ==
           0);
    ASSERT(keyset[i].weight() == 1.0F);
  }

  keyset.clear();
  total_length = 0;
  // Same thing again, but now via string_view, and with a weight.
  for (std::size_t i = 0; i < keys.size(); ++i) {
    keyset.push_back(keys[i], 2.0);
    ASSERT(keyset.size() == (i + 1));
    ASSERT(!keyset.empty());

    total_length += keys[i].length();
    ASSERT(keyset.total_length() == total_length);

    ASSERT(keyset[i].length() == keys[i].length());
    ASSERT(keyset[i].str().length() == keys[i].length());
    ASSERT(std::memcmp(keyset[i].ptr(), keys[i].c_str(), keyset[i].length()) ==
           0);
    ASSERT(keyset[i].str() == keys[i]);
    ASSERT(keyset[i].weight() == 2.0F);
  }

  keyset.clear();

  marisa::Key key;

  key.set_str("123");
  keyset.push_back(key);
  ASSERT(keyset[0].length() == 3);
  ASSERT(std::memcmp(keyset[0].ptr(), "123", 3) == 0);

  key.set_str("456");
  keyset.push_back(key, '\0');
  ASSERT(keyset[1].length() == 3);
  ASSERT(std::memcmp(keyset[1].ptr(), "456", 3) == 0);
  ASSERT(std::strcmp(keyset[1].ptr(), "456") == 0);

  key.set_str("789");
  keyset.push_back(key, '0');
  ASSERT(keyset[2].length() == 3);
  ASSERT(std::memcmp(keyset[2].ptr(), "789", 3) == 0);
  ASSERT(std::memcmp(keyset[2].ptr(), "7890", 4) == 0);

  ASSERT(keyset.size() == 3);

  keyset.clear();

  ASSERT(keyset.size() == 0);
  ASSERT(keyset.total_length() == 0);

  keys.resize(1000);
  std::vector<float> weights(keys.size());

  total_length = 0;
  for (std::size_t i = 0; i < keys.size(); ++i) {
    keys[i].resize(random_engine() % (marisa::Keyset::EXTRA_BLOCK_SIZE * 2));
    for (std::size_t j = 0; j < keys[i].length(); ++j) {
      keys[i][j] = static_cast<char>(random_engine() & 0xFF);
    }
    double weight = 100.0 * static_cast<double>(random_engine()) /
                    static_cast<double>(RAND_MAX);
    weights[i] = static_cast<float>(weight);

    keyset.push_back(keys[i].c_str(), keys[i].length(), weights[i]);
    total_length += keys[i].length();
    ASSERT(keyset.total_length() == total_length);
  }

  ASSERT(keyset.size() == keys.size());
  for (std::size_t i = 0; i < keys.size(); ++i) {
    ASSERT(keyset[i].length() == keys[i].length());
    ASSERT(std::memcmp(keyset[i].ptr(), keys[i].c_str(), keyset[i].length()) ==
           0);
    ASSERT(keyset[i].weight() == weights[i]);
  }

  keyset.reset();

  ASSERT(keyset.size() == 0);
  ASSERT(keyset.total_length() == 0);

  total_length = 0;
  for (std::size_t i = 0; i < keys.size(); ++i) {
    keys[i].resize(random_engine() % (marisa::Keyset::EXTRA_BLOCK_SIZE * 2));
    for (std::size_t j = 0; j < keys[i].length(); ++j) {
      keys[i][j] = static_cast<char>(random_engine() & 0xFF);
    }
    double weight = 100.0 * static_cast<double>(random_engine()) /
                    static_cast<double>(RAND_MAX);
    weights[i] = static_cast<float>(weight);

    keyset.push_back(keys[i].c_str(), keys[i].length(), weights[i]);
    total_length += keys[i].length();
    ASSERT(keyset.total_length() == total_length);
  }

  ASSERT(keyset.size() == keys.size());
  for (std::size_t i = 0; i < keys.size(); ++i) {
    ASSERT(keyset[i].length() == keys[i].length());
    ASSERT(std::memcmp(keyset[i].ptr(), keys[i].c_str(), keyset[i].length()) ==
           0);
    ASSERT(keyset[i].weight() == weights[i]);
  }

  TEST_END();
}

void TestQuery() {
  TEST_START();

  marisa::Query query;

  ASSERT(query.ptr() == nullptr);
  ASSERT(query.length() == 0);
  ASSERT(query.id() == 0);

  const char *str = "apple";
  query.set_str(str);

  ASSERT(query.ptr() == str);
  ASSERT(query.length() == std::strlen(str));

  query.set_str(str, 3);

  ASSERT(query.ptr() == str);
  ASSERT(query.length() == 3);

  const std::string_view view = "orange";
  query.set_str(view);

  ASSERT(query.ptr() == view.data());
  ASSERT(query.length() == 6);

  // Compare string_view, emphasizing that it is not a pointer comparison.
  ASSERT(query.str() == std::string("orange"));

  query.set_id(100);

  ASSERT(query.id() == 100);

  query.clear();

  ASSERT(query.ptr() == nullptr);
  ASSERT(query.length() == 0);
  ASSERT(query.id() == 0);

  TEST_END();
}

void TestAgent() {
  TEST_START();

  marisa::Agent agent;

  ASSERT(agent.query().ptr() == nullptr);
  ASSERT(agent.query().length() == 0);
  ASSERT(agent.query().id() == 0);

  ASSERT(agent.key().ptr() == nullptr);
  ASSERT(agent.key().length() == 0);

  ASSERT(!agent.has_state());

  const char *query_str = "query";
  const char *key_str = "key";

  agent.set_query(query_str);
  agent.set_query(123);
  agent.set_key(key_str);
  agent.set_key(234);

  ASSERT(agent.query().ptr() == query_str);
  ASSERT(agent.query().length() == std::strlen(query_str));
  ASSERT(agent.query().id() == 123);

  ASSERT(agent.key().ptr() == key_str);
  ASSERT(agent.key().length() == std::strlen(key_str));
  ASSERT(agent.key().id() == 234);

  const std::string_view query_view = "query2";
  const std::string_view key_view = "key2";

  agent.set_query(query_view);
  agent.set_key(key_view);

  ASSERT(agent.query().ptr() == query_view.data());
  ASSERT(agent.query().length() == 6);
  // Compare string_view, emphasizing that it is not a pointer comparison.
  ASSERT(agent.query().str() == std::string("query2"));

  ASSERT(agent.key().ptr() == key_view.data());
  ASSERT(agent.key().length() == 4);
  ASSERT(agent.key().str() == std::string("key2"));

  agent.init_state();

  ASSERT(agent.has_state());

  EXCEPT(agent.init_state(), std::logic_error);

  agent.clear();

  ASSERT(agent.query().ptr() == nullptr);
  ASSERT(agent.query().length() == 0);
  ASSERT(agent.query().id() == 0);

  ASSERT(agent.key().ptr() == nullptr);
  ASSERT(agent.key().length() == 0);

  ASSERT(!agent.has_state());

  TEST_END();
}

}  // namespace

int main() try {
  TestSwap();
  TestException();
  TestKey();
  TestKeyset();
  TestQuery();
  TestAgent();

  return 0;
} catch (const std::exception &ex) {
  std::cerr << ex.what() << "\n";
  throw;
}
