#include <marisa.h>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "cmdopt.h"

namespace {

int param_min_num_tries = 1;
int param_max_num_tries = 5;
marisa::TailMode param_tail_mode = MARISA_DEFAULT_TAIL;
marisa::NodeOrder param_node_order = MARISA_DEFAULT_ORDER;
marisa::CacheLevel param_cache_level = MARISA_DEFAULT_CACHE;
bool param_predict_on = true;
bool param_reuse_on = true;
bool param_print_speed = true;

class Clock {
 public:
  Clock() : cl_(std::clock()) {}

  void reset() {
    cl_ = std::clock();
  }

  double elasped() const {
    std::clock_t cur = std::clock();
    return static_cast<double>(cur - cl_) / static_cast<double>(CLOCKS_PER_SEC);
  }

 private:
  std::clock_t cl_;
};

void print_help(const char *cmd) {
  std::cerr
      << "Usage: " << cmd
      << " [OPTION]... [FILE]...\n\n"
         "Options:\n"
         "  -N, --min-num-tries=[N]  limit the number of tries ["
      << MARISA_MIN_NUM_TRIES << ", " << MARISA_MAX_NUM_TRIES
      << "] (default: 1)\n"
         "  -n, --max-num-tries=[N]  limit the number of tries ["
      << MARISA_MIN_NUM_TRIES << ", " << MARISA_MAX_NUM_TRIES
      << "] (default: 5)\n"
         "  -t, --text-tail     build a dictionary with text TAIL (default)\n"
         "  -b, --binary-tail   build a dictionary with binary TAIL\n"
         "  -w, --weight-order  arrange siblings in weight order (default)\n"
         "  -l, --label-order   arrange siblings in label order\n"
         "  -c, --cache-level=[N]    specify the cache size"
         " [1, 5] (default: 3)\n"
         "  -P, --predict-on    include predictive search (default)\n"
         "  -p, --predict-off   skip predictive search\n"
         "  -R, --reuse-on      reuse agents (default)\n"
         "  -r, --reuse-off     don't reuse agents\n"
         "  -S, --print-speed   print speed [1000 keys/s] (default)\n"
         "  -s, --print-time    print time [ns/key]\n"
         "  -h, --help          print this help\n"
         "\n";
}

void print_config() {
  std::cout << "Number of tries: " << param_min_num_tries << " - "
            << param_max_num_tries << "\n";

  std::cout << "TAIL mode: ";
  switch (param_tail_mode) {
    case MARISA_TEXT_TAIL: {
      std::cout << "Text mode\n";
      break;
    }
    case MARISA_BINARY_TAIL: {
      std::cout << "Binary mode\n";
      break;
    }
  }

  std::cout << "Node order: ";
  switch (param_node_order) {
    case MARISA_LABEL_ORDER: {
      std::cout << "Ascending label order\n";
      break;
    }
    case MARISA_WEIGHT_ORDER: {
      std::cout << "Descending weight order\n";
      break;
    }
  }

  std::cout << "Cache level: ";
  switch (param_cache_level) {
    case MARISA_HUGE_CACHE: {
      std::cout << "Huge cache\n";
      break;
    }
    case MARISA_LARGE_CACHE: {
      std::cout << "Large cache\n";
      break;
    }
    case MARISA_NORMAL_CACHE: {
      std::cout << "Normal cache\n";
      break;
    }
    case MARISA_SMALL_CACHE: {
      std::cout << "Small cache\n";
      break;
    }
    case MARISA_TINY_CACHE: {
      std::cout << "Tiny cache\n";
      break;
    }
  }
}

void print_time_info(std::size_t num_keys, double elasped) {
  if (param_print_speed) {
    if (elasped == 0.0) {
      std::printf(" %8s", "-");
    } else {
      std::printf(" %8.2f", static_cast<double>(num_keys) / elasped / 1000.0);
    }
  } else {
    if ((elasped == 0.0) || (num_keys == 0)) {
      std::printf(" %8s", "-");
    } else {
      std::printf(" %8.1f",
                  1000000000.0 * elasped / static_cast<double>(num_keys));
    }
  }
}

void read_keys(std::istream &input, marisa::Keyset *keyset,
               std::vector<float> *weights) {
  std::string line;
  while (std::getline(input, line)) {
    const std::string::size_type delim_pos = line.find_last_of('\t');
    float weight = 1.0F;
    if (delim_pos != line.npos) {
      char *end_of_value;
      weight =
          static_cast<float>(std::strtod(&line[delim_pos + 1], &end_of_value));
      if (*end_of_value == '\0') {
        line.resize(delim_pos);
      }
    }
    keyset->push_back(line.c_str(), line.length());
    weights->push_back(weight);
  }
}

int read_keys(const char *const *args, std::size_t num_args,
              marisa::Keyset *keyset, std::vector<float> *weights) {
  if (num_args == 0) {
    read_keys(std::cin, keyset, weights);
  }
  for (std::size_t i = 0; i < num_args; ++i) {
    std::ifstream input_file(args[i], std::ios::binary);
    if (!input_file) {
      std::cerr << "error: failed to open: " << args[i] << "\n";
      return 10;
    }
    read_keys(input_file, keyset, weights);
  }
  std::cout << "Number of keys: " << keyset->size() << "\n";
  std::cout << "Total length: " << keyset->total_length() << "\n" << std::flush;
  return 0;
}

void benchmark_build(marisa::Keyset &keyset, const std::vector<float> &weights,
                     int num_tries, marisa::Trie *trie) {
  for (std::size_t i = 0; i < keyset.size(); ++i) {
    keyset[i].set_weight(weights[i]);
  }
  Clock cl;
  trie->build(keyset, num_tries | param_tail_mode | param_node_order |
                          param_cache_level);
  std::printf(" %10lu", static_cast<unsigned long>(trie->io_size()));
  print_time_info(keyset.size(), cl.elasped());
}

void benchmark_lookup(const marisa::Trie &trie, const marisa::Keyset &keyset) {
  Clock cl;
  if (param_reuse_on) {
    marisa::Agent agent;
    for (std::size_t i = 0; i < keyset.size(); ++i) {
      agent.set_query(keyset[i].ptr(), keyset[i].length());
      if (!trie.lookup(agent) || (agent.key().id() != keyset[i].id())) {
        std::cerr << "error: lookup() failed\n";
        return;
      }
    }
  } else {
    for (std::size_t i = 0; i < keyset.size(); ++i) {
      marisa::Agent agent;
      agent.set_query(keyset[i].ptr(), keyset[i].length());
      if (!trie.lookup(agent) || (agent.key().id() != keyset[i].id())) {
        std::cerr << "error: lookup() failed\n";
        return;
      }
    }
  }
  print_time_info(keyset.size(), cl.elasped());
}

void benchmark_reverse_lookup(const marisa::Trie &trie,
                              const marisa::Keyset &keyset) {
  Clock cl;
  if (param_reuse_on) {
    marisa::Agent agent;
    for (std::size_t i = 0; i < keyset.size(); ++i) {
      agent.set_query(keyset[i].id());
      trie.reverse_lookup(agent);
      if ((agent.key().id() != keyset[i].id()) ||
          (agent.key().length() != keyset[i].length()) ||
          (std::memcmp(agent.key().ptr(), keyset[i].ptr(),
                       agent.key().length()) != 0)) {
        std::cerr << "error: reverse_lookup() failed\n";
        return;
      }
    }
  } else {
    for (std::size_t i = 0; i < keyset.size(); ++i) {
      marisa::Agent agent;
      agent.set_query(keyset[i].id());
      trie.reverse_lookup(agent);
      if ((agent.key().id() != keyset[i].id()) ||
          (agent.key().length() != keyset[i].length()) ||
          (std::memcmp(agent.key().ptr(), keyset[i].ptr(),
                       agent.key().length()) != 0)) {
        std::cerr << "error: reverse_lookup() failed\n";
        return;
      }
    }
  }
  print_time_info(keyset.size(), cl.elasped());
}

void benchmark_common_prefix_search(const marisa::Trie &trie,
                                    const marisa::Keyset &keyset) {
  Clock cl;
  if (param_reuse_on) {
    marisa::Agent agent;
    for (std::size_t i = 0; i < keyset.size(); ++i) {
      agent.set_query(keyset[i].ptr(), keyset[i].length());
      while (trie.common_prefix_search(agent)) {
        if (agent.key().id() > keyset[i].id()) {
          std::cerr << "error: common_prefix_search() failed\n";
          return;
        }
      }
      if (agent.key().id() != keyset[i].id()) {
        std::cerr << "error: common_prefix_search() failed\n";
        return;
      }
    }
  } else {
    for (std::size_t i = 0; i < keyset.size(); ++i) {
      marisa::Agent agent;
      agent.set_query(keyset[i].ptr(), keyset[i].length());
      while (trie.common_prefix_search(agent)) {
        if (agent.key().id() > keyset[i].id()) {
          std::cerr << "error: common_prefix_search() failed\n";
          return;
        }
      }
      if (agent.key().id() != keyset[i].id()) {
        std::cerr << "error: common_prefix_search() failed\n";
        return;
      }
    }
  }
  print_time_info(keyset.size(), cl.elasped());
}

void benchmark_predictive_search(const marisa::Trie &trie,
                                 const marisa::Keyset &keyset) {
  if (!param_predict_on) {
    print_time_info(keyset.size(), 0.0);
    return;
  }

  Clock cl;
  if (param_reuse_on) {
    marisa::Agent agent;
    for (std::size_t i = 0; i < keyset.size(); ++i) {
      agent.set_query(keyset[i].ptr(), keyset[i].length());
      if (!trie.predictive_search(agent) ||
          (agent.key().id() != keyset[i].id())) {
        std::cerr << "error: predictive_search() failed\n";
        return;
      }
      while (trie.predictive_search(agent)) {
        if (agent.key().id() <= keyset[i].id()) {
          std::cerr << "error: predictive_search() failed\n";
          return;
        }
      }
    }
  } else {
    for (std::size_t i = 0; i < keyset.size(); ++i) {
      marisa::Agent agent;
      agent.set_query(keyset[i].ptr(), keyset[i].length());
      if (!trie.predictive_search(agent) ||
          (agent.key().id() != keyset[i].id())) {
        std::cerr << "error: predictive_search() failed\n";
        return;
      }
      while (trie.predictive_search(agent)) {
        if (agent.key().id() <= keyset[i].id()) {
          std::cerr << "error: predictive_search() failed\n";
          return;
        }
      }
    }
  }
  print_time_info(keyset.size(), cl.elasped());
}

void benchmark(marisa::Keyset &keyset, const std::vector<float> &weights,
               int num_tries) {
  std::printf("%6d", num_tries);
  marisa::Trie trie;
  benchmark_build(keyset, weights, num_tries, &trie);
  if (!trie.empty()) {
    benchmark_lookup(trie, keyset);
    benchmark_reverse_lookup(trie, keyset);
    benchmark_common_prefix_search(trie, keyset);
    benchmark_predictive_search(trie, keyset);
  }
  std::printf("\n");
}

int benchmark(const char *const *args, std::size_t num_args) try {
  marisa::Keyset keyset;
  std::vector<float> weights;
  const int ret = read_keys(args, num_args, &keyset, &weights);
  if (ret != 0) {
    return ret;
  }
  std::printf(
      "------+----------+--------+--------+--------+--------+--------\n");
  std::printf("%6s %10s %8s %8s %8s %8s %8s\n", "#tries", "size", "build",
              "lookup", "reverse", "prefix", "predict");
  std::printf("%6s %10s %8s %8s %8s %8s %8s\n", "", "", "", "", "lookup",
              "search", "search");
  if (param_print_speed) {
    std::printf("%6s %10s %8s %8s %8s %8s %8s\n", "", "[bytes]", "[K/s]",
                "[K/s]", "[K/s]", "[K/s]", "[K/s]");
  } else {
    std::printf("%6s %10s %8s %8s %8s %8s %8s\n", "", "[bytes]", "[ns]", "[ns]",
                "[ns]", "[ns]", "[ns]");
  }
  std::printf(
      "------+----------+--------+--------+--------+--------+--------\n");
  for (int i = param_min_num_tries; i <= param_max_num_tries; ++i) {
    benchmark(keyset, weights, i);
  }
  std::printf(
      "------+----------+--------+--------+--------+--------+--------\n");
  return 0;
} catch (const std::exception &ex) {
  std::cerr << ex.what() << "\n";
  return -1;
}

}  // namespace

int main(int argc, char *argv[]) {
  std::ios::sync_with_stdio(false);

  ::cmdopt_option long_options[] = {{"min-num-tries", 1, nullptr, 'N'},
                                    {"max-num-tries", 1, nullptr, 'n'},
                                    {"text-tail", 0, nullptr, 't'},
                                    {"binary-tail", 0, nullptr, 'b'},
                                    {"weight-order", 0, nullptr, 'w'},
                                    {"label-order", 0, nullptr, 'l'},
                                    {"cache-level", 1, nullptr, 'c'},
                                    {"predict-on", 0, nullptr, 'P'},
                                    {"predict-off", 0, nullptr, 'p'},
                                    {"reuse-on", 0, nullptr, 'R'},
                                    {"reuse-off", 0, nullptr, 'r'},
                                    {"print-speed", 0, nullptr, 'S'},
                                    {"print-time", 0, nullptr, 's'},
                                    {"help", 0, nullptr, 'h'},
                                    {nullptr, 0, nullptr, 0}};
  ::cmdopt_t cmdopt;
  ::cmdopt_init(&cmdopt, argc, argv, "N:n:tbwlc:PpRrSsh", long_options);
  int label;
  while ((label = ::cmdopt_get(&cmdopt)) != -1) {
    switch (label) {
      case 'N': {
        char *end_of_value;
        const long value = std::strtol(cmdopt.optarg, &end_of_value, 10);
        if ((*end_of_value != '\0') || (value <= 0) ||
            (value > MARISA_MAX_NUM_TRIES)) {
          std::cerr << "error: option `-n' with an invalid argument: "
                    << cmdopt.optarg << "\n";
          return 1;
        }
        param_min_num_tries = static_cast<int>(value);
        break;
      }
      case 'n': {
        char *end_of_value;
        const long value = std::strtol(cmdopt.optarg, &end_of_value, 10);
        if ((*end_of_value != '\0') || (value <= 0) ||
            (value > MARISA_MAX_NUM_TRIES)) {
          std::cerr << "error: option `-n' with an invalid argument: "
                    << cmdopt.optarg << "\n";
          return 2;
        }
        param_max_num_tries = static_cast<int>(value);
        break;
      }
      case 't': {
        param_tail_mode = MARISA_TEXT_TAIL;
        break;
      }
      case 'b': {
        param_tail_mode = MARISA_BINARY_TAIL;
        break;
      }
      case 'w': {
        param_node_order = MARISA_WEIGHT_ORDER;
        break;
      }
      case 'l': {
        param_node_order = MARISA_LABEL_ORDER;
        break;
      }
      case 'c': {
        char *end_of_value;
        const long value = std::strtol(cmdopt.optarg, &end_of_value, 10);
        if ((*end_of_value != '\0') || (value < 1) || (value > 5)) {
          std::cerr << "error: option `-c' with an invalid argument: "
                    << cmdopt.optarg << "\n";
          return 3;
        }
        if (value == 1) {
          param_cache_level = MARISA_TINY_CACHE;
        } else if (value == 2) {
          param_cache_level = MARISA_SMALL_CACHE;
        } else if (value == 3) {
          param_cache_level = MARISA_NORMAL_CACHE;
        } else if (value == 4) {
          param_cache_level = MARISA_LARGE_CACHE;
        } else if (value == 5) {
          param_cache_level = MARISA_HUGE_CACHE;
        }
        break;
      }
      case 'P': {
        param_predict_on = true;
        break;
      }
      case 'p': {
        param_predict_on = false;
        break;
      }
      case 'R': {
        param_reuse_on = true;
        break;
      }
      case 'r': {
        param_reuse_on = false;
        break;
      }
      case 'S': {
        param_print_speed = true;
        break;
      }
      case 's': {
        param_print_speed = false;
        break;
      }
      case 'h': {
        print_help(argv[0]);
        return 0;
      }
      default: {
        return 1;
      }
    }
  }
  print_config();
  return benchmark(cmdopt.argv + cmdopt.optind,
                   static_cast<std::size_t>(cmdopt.argc - cmdopt.optind));
}
