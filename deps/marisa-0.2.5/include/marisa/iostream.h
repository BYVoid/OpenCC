#ifndef MARISA_IOSTREAM_H_
#define MARISA_IOSTREAM_H_

#include <iosfwd>

namespace marisa {

class Trie;

std::istream &read(std::istream &stream, Trie *trie);
std::ostream &write(std::ostream &stream, const Trie &trie);

std::istream &operator>>(std::istream &stream, Trie &trie);
std::ostream &operator<<(std::ostream &stream, const Trie &trie);

}  // namespace marisa

#endif  // MARISA_IOSTREAM_H_
