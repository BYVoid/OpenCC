#include <marisa.h>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#include "cmdopt.h"

namespace {

std::size_t max_num_results = 10;
bool mmap_flag = true;

void print_help(const char *cmd) {
  std::cerr
      << "Usage: " << cmd
      << " [OPTION]... DIC\n\n"
         "Options:\n"
         "  -n, --max-num-results=[N]  limit the number of outputs to N"
         " (default: 10)\n"
         "                             0: no limit\n"
         "  -m, --mmap-dictionary  use memory-mapped I/O to load a dictionary"
         " (default)\n"
         "  -r, --read-dictionary  read an entire dictionary into memory\n"
         "  -h, --help             print this help\n"
         "\n";
}

int predictive_search(const char *const *args, std::size_t num_args) {
  if (num_args == 0) {
    std::cerr << "error: dictionary is not specified\n";
    return 10;
  }
  if (num_args > 1) {
    std::cerr << "error: more than one dictionaries are specified\n";
    return 11;
  }

  marisa::Trie trie;
  if (mmap_flag) {
    try {
      trie.mmap(args[0]);
    } catch (const std::exception &ex) {
      std::cerr << ex.what()
                << ": failed to mmap a dictionary file: " << args[0] << "\n";
      return 20;
    }
  } else {
    try {
      trie.load(args[0]);
    } catch (const std::exception &ex) {
      std::cerr << ex.what()
                << ": failed to load a dictionary file: " << args[0] << "\n";
      return 21;
    }
  }

  marisa::Agent agent;
  marisa::Keyset keyset;
  std::string str;
  while (std::getline(std::cin, str)) {
    try {
      agent.set_query(str.c_str(), str.length());
      while (trie.predictive_search(agent)) {
        keyset.push_back(agent.key());
      }
      if (keyset.empty()) {
        std::cout << "not found\n";
      } else {
        std::cout << keyset.size() << " found\n";
        const std::size_t end = std::min(max_num_results, keyset.size());
        for (std::size_t i = 0; i < end; ++i) {
          std::cout << keyset[i].id() << '\t';
          std::cout.write(keyset[i].ptr(),
                          static_cast<std::streamsize>(keyset[i].length()))
              << '\t';
          std::cout << str << '\n';
        }
      }
      keyset.reset();
    } catch (const std::exception &ex) {
      std::cerr << ex.what() << ": predictive_search() failed: " << str << "\n";
      return 30;
    }

    if (!std::cout) {
      std::cerr << "error: failed to write results to standard output\n";
      return 31;
    }
  }

  return 0;
}

}  // namespace

int main(int argc, char *argv[]) {
  std::ios::sync_with_stdio(false);

  ::cmdopt_option long_options[] = {{"max-num-results", 1, nullptr, 'n'},
                                    {"mmap-dictionary", 0, nullptr, 'm'},
                                    {"read-dictionary", 0, nullptr, 'r'},
                                    {"help", 0, nullptr, 'h'},
                                    {nullptr, 0, nullptr, 0}};
  ::cmdopt_t cmdopt;
  ::cmdopt_init(&cmdopt, argc, argv, "n:mrh", long_options);
  int label;
  while ((label = ::cmdopt_get(&cmdopt)) != -1) {
    switch (label) {
      case 'n': {
        char *end_of_value;
        const long value = std::strtol(cmdopt.optarg, &end_of_value, 10);
        if ((*end_of_value != '\0') || (value < 0)) {
          std::cerr << "error: option `-n' with an invalid argument: "
                    << cmdopt.optarg << "\n";
        }
        if ((value == 0) ||
            (static_cast<unsigned long long>(value) > SIZE_MAX)) {
          max_num_results = SIZE_MAX;
        } else {
          max_num_results = static_cast<std::size_t>(value);
        }
        break;
      }
      case 'm': {
        mmap_flag = true;
        break;
      }
      case 'r': {
        mmap_flag = false;
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
  return predictive_search(
      cmdopt.argv + cmdopt.optind,
      static_cast<std::size_t>(cmdopt.argc - cmdopt.optind));
}
