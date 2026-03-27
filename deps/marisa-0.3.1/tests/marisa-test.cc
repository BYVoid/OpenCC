#include <marisa.h>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "marisa-assert.h"

namespace {

std::random_device seed_gen;
std::mt19937 random_engine(seed_gen());

void TestEmptyTrie() {
  TEST_START();

  marisa::Trie trie;

  EXCEPT(trie.save("marisa-test.dat"), std::logic_error);
#ifdef _MSC_VER
  EXCEPT(trie.write(::_fileno(stdout)), std::logic_error);
#else   // _MSC_VER
  EXCEPT(trie.write(::fileno(stdout)), std::logic_error);
#endif  // _MSC_VER
  EXCEPT(std::cout << trie, std::logic_error);
  EXCEPT(marisa::fwrite(stdout, trie), std::logic_error);

  marisa::Agent agent;

  EXCEPT(trie.lookup(agent), std::logic_error);
  EXCEPT(trie.reverse_lookup(agent), std::logic_error);
  EXCEPT(trie.common_prefix_search(agent), std::logic_error);
  EXCEPT(trie.predictive_search(agent), std::logic_error);

  EXCEPT(trie.num_tries(), std::logic_error);
  EXCEPT(trie.num_keys(), std::logic_error);
  EXCEPT(trie.num_nodes(), std::logic_error);

  EXCEPT(trie.tail_mode(), std::logic_error);
  EXCEPT(trie.node_order(), std::logic_error);

  EXCEPT(trie.empty(), std::logic_error);
  EXCEPT(trie.size(), std::logic_error);
  EXCEPT(trie.total_size(), std::logic_error);
  EXCEPT(trie.io_size(), std::logic_error);

  marisa::Keyset keyset;
  trie.build(keyset);

  ASSERT(!trie.lookup(agent));
  EXCEPT(trie.reverse_lookup(agent), std::out_of_range);
  ASSERT(!trie.common_prefix_search(agent));
  ASSERT(!trie.predictive_search(agent));

  ASSERT(trie.num_tries() == 1);
  ASSERT(trie.num_keys() == 0);
  ASSERT(trie.num_nodes() == 1);

  ASSERT(trie.tail_mode() == MARISA_DEFAULT_TAIL);
  ASSERT(trie.node_order() == MARISA_DEFAULT_ORDER);

  ASSERT(trie.empty());
  ASSERT(trie.size() == 0);
  ASSERT(trie.total_size() != 0);
  ASSERT(trie.io_size() != 0);

  keyset.push_back("");
  trie.build(keyset);

  ASSERT(trie.lookup(agent));
  trie.reverse_lookup(agent);
  ASSERT(trie.common_prefix_search(agent));
  ASSERT(!trie.common_prefix_search(agent));
  ASSERT(trie.predictive_search(agent));
  ASSERT(!trie.predictive_search(agent));

  ASSERT(trie.num_keys() == 1);
  ASSERT(trie.num_nodes() == 1);

  ASSERT(!trie.empty());
  ASSERT(trie.size() == 1);
  ASSERT(trie.total_size() != 0);
  ASSERT(trie.io_size() != 0);

  TEST_END();
}

void TestTinyTrie() {
  TEST_START();

  marisa::Keyset keyset;
  keyset.push_back("bach");
  keyset.push_back("bet");
  keyset.push_back("chat");
  keyset.push_back("check");
  keyset.push_back("check");

  marisa::Trie trie;
  trie.build(keyset, 1);

  ASSERT(trie.num_tries() == 1);
  ASSERT(trie.num_keys() == 4);
  ASSERT(trie.num_nodes() == 7);

  ASSERT(trie.tail_mode() == MARISA_DEFAULT_TAIL);
  ASSERT(trie.node_order() == MARISA_DEFAULT_ORDER);

  ASSERT(keyset[0].id() == 2);
  ASSERT(keyset[1].id() == 3);
  ASSERT(keyset[2].id() == 1);
  ASSERT(keyset[3].id() == 0);
  ASSERT(keyset[4].id() == 0);

  marisa::Agent agent;
  for (std::size_t i = 0; i < keyset.size(); ++i) {
    agent.set_query(keyset[i].ptr(), keyset[i].length());
    ASSERT(trie.lookup(agent));
    ASSERT(agent.key().id() == keyset[i].id());

    agent.set_query(keyset[i].id());
    trie.reverse_lookup(agent);
    ASSERT(agent.key().length() == keyset[i].length());
    ASSERT(std::memcmp(agent.key().ptr(), keyset[i].ptr(),
                       agent.key().length()) == 0);
  }

  agent.set_query("be");
  ASSERT(!trie.common_prefix_search(agent));
  agent.set_query("beX");
  ASSERT(!trie.common_prefix_search(agent));
  agent.set_query("bet");
  ASSERT(trie.common_prefix_search(agent));
  ASSERT(!trie.common_prefix_search(agent));
  agent.set_query("betX");
  ASSERT(trie.common_prefix_search(agent));
  ASSERT(!trie.common_prefix_search(agent));

  agent.set_query("chatX");
  ASSERT(!trie.predictive_search(agent));
  agent.set_query("chat");
  ASSERT(trie.predictive_search(agent));
  ASSERT(agent.key().length() == 4);
  ASSERT(!trie.predictive_search(agent));

  agent.set_query("cha");
  ASSERT(trie.predictive_search(agent));
  ASSERT(agent.key().length() == 4);
  ASSERT(!trie.predictive_search(agent));

  agent.set_query("c");
  ASSERT(trie.predictive_search(agent));
  ASSERT(agent.key().length() == 5);
  ASSERT(std::memcmp(agent.key().ptr(), "check", 5) == 0);
  ASSERT(trie.predictive_search(agent));
  ASSERT(agent.key().length() == 4);
  ASSERT(std::memcmp(agent.key().ptr(), "chat", 4) == 0);
  ASSERT(!trie.predictive_search(agent));

  agent.set_query("ch");
  ASSERT(trie.predictive_search(agent));
  ASSERT(agent.key().length() == 5);
  ASSERT(std::memcmp(agent.key().ptr(), "check", 5) == 0);
  ASSERT(trie.predictive_search(agent));
  ASSERT(agent.key().length() == 4);
  ASSERT(std::memcmp(agent.key().ptr(), "chat", 4) == 0);
  ASSERT(!trie.predictive_search(agent));

  trie.build(keyset, 1 | MARISA_LABEL_ORDER);

  ASSERT(trie.num_tries() == 1);
  ASSERT(trie.num_keys() == 4);
  ASSERT(trie.num_nodes() == 7);

  ASSERT(trie.tail_mode() == MARISA_DEFAULT_TAIL);
  ASSERT(trie.node_order() == MARISA_LABEL_ORDER);

  ASSERT(keyset[0].id() == 0);
  ASSERT(keyset[1].id() == 1);
  ASSERT(keyset[2].id() == 2);
  ASSERT(keyset[3].id() == 3);
  ASSERT(keyset[4].id() == 3);

  for (std::size_t i = 0; i < keyset.size(); ++i) {
    agent.set_query(keyset[i].ptr(), keyset[i].length());
    ASSERT(trie.lookup(agent));
    ASSERT(agent.key().id() == keyset[i].id());

    agent.set_query(keyset[i].id());
    trie.reverse_lookup(agent);
    ASSERT(agent.key().length() == keyset[i].length());
    ASSERT(std::memcmp(agent.key().ptr(), keyset[i].ptr(),
                       agent.key().length()) == 0);
  }

  agent.set_query("");
  for (std::size_t i = 0; i < trie.size(); ++i) {
    ASSERT(trie.predictive_search(agent));
    ASSERT(agent.key().id() == i);
  }
  ASSERT(!trie.predictive_search(agent));

  TEST_END();
}

void MakeKeyset(std::size_t num_keys, marisa::TailMode tail_mode,
                marisa::Keyset *keyset) {
  char key_buf[16];
  for (std::size_t i = 0; i < num_keys; ++i) {
    const std::size_t length =
        static_cast<std::size_t>(random_engine()) % sizeof(key_buf);
    for (std::size_t j = 0; j < length; ++j) {
      key_buf[j] = static_cast<char>(random_engine() % 10);
      if (tail_mode == MARISA_TEXT_TAIL) {
        key_buf[j] = static_cast<char>(key_buf[j] + '0');
      }
    }
    keyset->push_back(key_buf, length);
  }
}

void TestLookup(const marisa::Trie &trie, const marisa::Keyset &keyset) {
  marisa::Agent agent;
  for (std::size_t i = 0; i < keyset.size(); ++i) {
    agent.set_query(keyset[i].ptr(), keyset[i].length());
    ASSERT(trie.lookup(agent));
    ASSERT(agent.key().id() == keyset[i].id());

    agent.set_query(keyset[i].id());
    trie.reverse_lookup(agent);
    ASSERT(agent.key().length() == keyset[i].length());
    ASSERT(std::memcmp(agent.key().ptr(), keyset[i].ptr(),
                       agent.key().length()) == 0);
  }
}

void TestCommonPrefixSearch(const marisa::Trie &trie,
                            const marisa::Keyset &keyset) {
  marisa::Agent agent;
  for (std::size_t i = 0; i < keyset.size(); ++i) {
    agent.set_query(keyset[i].ptr(), keyset[i].length());
    ASSERT(trie.common_prefix_search(agent));
    ASSERT(agent.key().id() <= keyset[i].id());
    while (trie.common_prefix_search(agent)) {
      ASSERT(agent.key().id() <= keyset[i].id());
    }
    ASSERT(agent.key().id() == keyset[i].id());
  }
}

void TestCommonPrefixSearchAgentCopy(const marisa::Trie &trie,
                                     const marisa::Keyset &keyset) {
  if (keyset.empty()) return;
  marisa::Agent agent;
  agent.set_query(keyset[0].ptr(), keyset[0].length());
  ASSERT(trie.common_prefix_search(agent));
  const std::string original_agent_key(agent.key().ptr(), agent.key().length());
  marisa::Agent agent_copy = agent;
  trie.common_prefix_search(agent);
  ASSERT(std::string(agent_copy.key().ptr(), agent_copy.key().length()) ==
         original_agent_key);
}

void TestPredictiveSearch(const marisa::Trie &trie,
                          const marisa::Keyset &keyset) {
  marisa::Agent agent;
  for (std::size_t i = 0; i < keyset.size(); ++i) {
    agent.set_query(keyset[i].ptr(), keyset[i].length());
    ASSERT(trie.predictive_search(agent));
    ASSERT(agent.key().id() == keyset[i].id());
    while (trie.predictive_search(agent)) {
      ASSERT(agent.key().id() > keyset[i].id());
    }
  }
}

void TestPredictiveSearchAgentCopy(const marisa::Trie &trie,
                                   const marisa::Keyset &keyset) {
  marisa::Agent agent;
  for (std::size_t i = 0; i < keyset.size(); ++i) {
    agent.set_query(keyset[i].ptr(), keyset[i].length());
    ASSERT(trie.predictive_search(agent));
    ASSERT(agent.key().id() == keyset[i].id());

    std::vector<marisa::Agent> agent_copies;
    std::vector<std::size_t> ids;
    std::vector<std::string> keys;
    while (trie.predictive_search(agent)) {
      ASSERT(agent.key().id() > keyset[i].id());
      ids.push_back(agent.key().id());
      keys.emplace_back(agent.key().ptr(), agent.key().length());

      // Tests copy constructor.
      agent_copies.push_back(agent);
    }

    for (std::size_t j = 0; j < agent_copies.size(); ++j) {
      marisa::Agent agent_copy;

      // Tests copy assignment.
      agent_copy = agent_copies[j];

      ASSERT(agent_copy.key().id() == ids[j]);
      ASSERT(std::string(agent_copy.key().ptr(), agent_copy.key().length()) ==
             keys[j]);
      if (j + 1 < agent_copies.size()) {
        ASSERT(trie.predictive_search(agent_copy));
        ASSERT(agent_copy.key().id() == ids[j + 1]);
        ASSERT(std::string(agent_copy.key().ptr(), agent_copy.key().length()) ==
               keys[j + 1]);
      } else {
        ASSERT(!trie.predictive_search(agent_copy));
      }
    }
  }
}

void TestPredictiveSearchAgentMove(const marisa::Trie &trie,
                                   const marisa::Keyset &keyset) {
  marisa::Agent agents[2];
  std::size_t current_agent = 0;

  const auto move_agent = [&]() {
    const std::size_t other_agent = (current_agent + 1) % 2;
    agents[other_agent] = std::move(agents[current_agent]);
    agents[current_agent] = {};
    current_agent = other_agent;
  };

  for (std::size_t i = 0; i < keyset.size(); ++i) {
    agents[current_agent].set_query(keyset[i].ptr(), keyset[i].length());
    move_agent();

    ASSERT(trie.predictive_search(agents[current_agent]));
    move_agent();

    ASSERT(agents[current_agent].key().id() == keyset[i].id());

    while (trie.predictive_search(agents[current_agent])) {
      move_agent();
      ASSERT(agents[current_agent].key().id() > keyset[i].id());
    }
  }
}

void TestTrie(int num_tries, marisa::TailMode tail_mode,
              marisa::NodeOrder node_order, marisa::Keyset &keyset) {
  for (std::size_t i = 0; i < keyset.size(); ++i) {
    keyset[i].set_weight(1.0F);
  }

  marisa::Trie trie;
  trie.build(keyset, num_tries | tail_mode | node_order);

  ASSERT(trie.num_tries() == static_cast<std::size_t>(num_tries));
  ASSERT(trie.num_keys() <= keyset.size());

  ASSERT(trie.tail_mode() == tail_mode);
  ASSERT(trie.node_order() == node_order);

  TestLookup(trie, keyset);
  TestCommonPrefixSearch(trie, keyset);
  TestCommonPrefixSearchAgentCopy(trie, keyset);
  TestPredictiveSearch(trie, keyset);
  TestPredictiveSearchAgentCopy(trie, keyset);
  TestPredictiveSearchAgentMove(trie, keyset);

  trie.save("marisa-test.dat");

  trie.clear();
  trie.load("marisa-test.dat");

  ASSERT(trie.num_tries() == static_cast<std::size_t>(num_tries));
  ASSERT(trie.num_keys() <= keyset.size());

  ASSERT(trie.tail_mode() == tail_mode);
  ASSERT(trie.node_order() == node_order);

  TestLookup(trie, keyset);

  {
    std::FILE *file;
#ifdef _MSC_VER
    ASSERT(::fopen_s(&file, "marisa-test.dat", "wb") == 0);
#else   // _MSC_VER
    file = std::fopen("marisa-test.dat", "wb");
    ASSERT(file != nullptr);
#endif  // _MSC_VER
    marisa::fwrite(file, trie);
    std::fclose(file);
    trie.clear();
#ifdef _MSC_VER
    ASSERT(::fopen_s(&file, "marisa-test.dat", "rb") == 0);
#else   // _MSC_VER
    file = std::fopen("marisa-test.dat", "rb");
    ASSERT(file != nullptr);
#endif  // _MSC_VER
    marisa::fread(file, &trie);
    std::fclose(file);
  }

  ASSERT(trie.num_tries() == static_cast<std::size_t>(num_tries));
  ASSERT(trie.num_keys() <= keyset.size());

  ASSERT(trie.tail_mode() == tail_mode);
  ASSERT(trie.node_order() == node_order);

  TestLookup(trie, keyset);

  trie.clear();
  trie.mmap("marisa-test.dat");

  ASSERT(trie.num_tries() == static_cast<std::size_t>(num_tries));
  ASSERT(trie.num_keys() <= keyset.size());

  ASSERT(trie.tail_mode() == tail_mode);
  ASSERT(trie.node_order() == node_order);

  TestLookup(trie, keyset);

  {
    std::stringstream stream;
    stream << trie;
    trie.clear();
    stream >> trie;
  }

  ASSERT(trie.num_tries() == static_cast<std::size_t>(num_tries));
  ASSERT(trie.num_keys() <= keyset.size());

  ASSERT(trie.tail_mode() == tail_mode);
  ASSERT(trie.node_order() == node_order);

  TestLookup(trie, keyset);
}

void TestTrie(marisa::TailMode tail_mode, marisa::NodeOrder node_order,
              marisa::Keyset &keyset) {
  TEST_START();
  std::cout << ((tail_mode == MARISA_TEXT_TAIL) ? "TEXT" : "BINARY") << ", ";
  std::cout << ((node_order == MARISA_WEIGHT_ORDER) ? "WEIGHT" : "LABEL")
            << ": ";

  for (int i = 1; i < 5; ++i) {
    TestTrie(i, tail_mode, node_order, keyset);
  }

  TEST_END();
}

void TestTrie(marisa::TailMode tail_mode) {
  marisa::Keyset keyset;
  MakeKeyset(1000, tail_mode, &keyset);

  TestTrie(tail_mode, MARISA_WEIGHT_ORDER, keyset);
  TestTrie(tail_mode, MARISA_LABEL_ORDER, keyset);
}

void TestTrie() {
  TestTrie(MARISA_TEXT_TAIL);
  TestTrie(MARISA_BINARY_TAIL);
}

}  // namespace

int main() try {
  TestEmptyTrie();
  TestTinyTrie();
  TestTrie();

  return 0;
} catch (const std::exception &ex) {
  std::cerr << ex.what() << "\n";
  throw;
}
