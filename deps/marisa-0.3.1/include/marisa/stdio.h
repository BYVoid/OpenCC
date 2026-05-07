#ifndef MARISA_MYSTDIO_H_
#define MARISA_MYSTDIO_H_

#include <cstdio>

namespace marisa {

class Trie;

void fread(std::FILE *file, Trie *trie);
void fwrite(std::FILE *file, const Trie &trie);

}  // namespace marisa

#endif  // MARISA_MYSTDIO_H_
