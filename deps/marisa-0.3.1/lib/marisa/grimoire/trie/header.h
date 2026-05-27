#ifndef MARISA_GRIMOIRE_TRIE_HEADER_H_
#define MARISA_GRIMOIRE_TRIE_HEADER_H_

#include <stdexcept>

#include "marisa/grimoire/io.h"

namespace marisa::grimoire::trie {

class Header {
 public:
  enum {
    HEADER_SIZE = 16
  };

  Header() = default;

  Header(const Header &) = delete;
  Header &operator=(const Header &) = delete;

  void map(Mapper &mapper) {
    const char *ptr;
    mapper.map(&ptr, HEADER_SIZE);
    MARISA_THROW_IF(!test_header(ptr), std::runtime_error);
  }
  void read(Reader &reader) {
    char buf[HEADER_SIZE];
    reader.read(buf, HEADER_SIZE);
    MARISA_THROW_IF(!test_header(buf), std::runtime_error);
  }
  void write(Writer &writer) const {
    writer.write(get_header(), HEADER_SIZE);
  }

  std::size_t io_size() const {
    return HEADER_SIZE;
  }

 private:
  static const char *get_header() {
    static const char buf[HEADER_SIZE] = "We love Marisa.";
    return buf;
  }

  static bool test_header(const char *ptr) {
    for (std::size_t i = 0; i < HEADER_SIZE; ++i) {
      if (ptr[i] != get_header()[i]) {
        return false;
      }
    }
    return true;
  }
};

}  // namespace marisa::grimoire::trie

#endif  // MARISA_GRIMOIRE_TRIE_HEADER_H_
