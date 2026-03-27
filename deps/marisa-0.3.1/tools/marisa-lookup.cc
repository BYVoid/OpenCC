#include <marisa.h>

#include <exception>
#include <iostream>
#include <string>

#include "cmdopt.h"

namespace {

bool mmap_flag = true;

void print_help(const char *cmd) {
  std::cerr
      << "Usage: " << cmd
      << " [OPTION]... DIC\n\n"
         "Options:\n"
         "  -m, --mmap-dictionary  use memory-mapped I/O to load a dictionary"
         " (default)\n"
         "  -r, --read-dictionary  read an entire dictionary into memory\n"
         "  -h, --help             print this help\n"
         "\n";
}

int lookup(const char *const *args, std::size_t num_args) {
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
  std::string str;
  while (std::getline(std::cin, str)) {
    try {
      agent.set_query(str.c_str(), str.length());
      if (trie.lookup(agent)) {
        std::cout << agent.key().id() << '\t' << str << '\n';
      } else {
        std::cout << "-1\t" << str << '\n';
      }
    } catch (const std::exception &ex) {
      std::cerr << ex.what() << ": lookup() failed: " << str << "\n";
      return 30;
    }

    if (!std::cout) {
      std::cerr << "error: failed to write results to standard output\n";
      return 30;
    }
  }

  return 0;
}

}  // namespace

int main(int argc, char *argv[]) {
  std::ios::sync_with_stdio(false);

  ::cmdopt_option long_options[] = {{"mmap-dictionary", 0, nullptr, 'm'},
                                    {"read-dictionary", 0, nullptr, 'r'},
                                    {"help", 0, nullptr, 'h'},
                                    {nullptr, 0, nullptr, 0}};
  ::cmdopt_t cmdopt;
  ::cmdopt_init(&cmdopt, argc, argv, "mrh", long_options);
  int label;
  while ((label = ::cmdopt_get(&cmdopt)) != -1) {
    switch (label) {
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
  return lookup(cmdopt.argv + cmdopt.optind,
                static_cast<std::size_t>(cmdopt.argc - cmdopt.optind));
}
