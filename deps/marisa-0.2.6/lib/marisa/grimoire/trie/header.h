#ifndef MARISA_GRIMOIRE_TRIE_HEADER_H_
#define MARISA_GRIMOIRE_TRIE_HEADER_H_

#include "marisa/grimoire/io.h"

namespace marisa {
namespace grimoire {
namespace trie {

class Header {
 public:
  enum {
    HEADER_SIZE = 16
  };

  Header() {}

  void map(Mapper &mapper) {
    const char *ptr;
    mapper.map(&ptr, HEADER_SIZE);
    MARISA_THROW_IF(!test_header(ptr), MARISA_FORMAT_ERROR);
  }
  void read(Reader &reader) {
    char buf[HEADER_SIZE];
    reader.read(buf, HEADER_SIZE);
    MARISA_THROW_IF(!test_header(buf), MARISA_FORMAT_ERROR);
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

  // Disallows copy and assignment.
  Header(const Header &);
  Header &operator=(const Header &);
};

}  // namespace trie
}  // namespace marisa
}  // namespace grimoire

#endif  // MARISA_GRIMOIRE_TRIE_HEADER_H_
