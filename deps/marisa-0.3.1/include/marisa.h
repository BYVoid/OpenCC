#ifndef MARISA_H_
#define MARISA_H_

// "marisa/stdio.h" includes <cstdio> for I/O using std::FILE.
#include "marisa/stdio.h"  // IWYU pragma: export

// "marisa/iostream.h" includes <iosfwd> for I/O using std::iostream.
#include "marisa/iostream.h"  // IWYU pragma: export

// You can use <marisa/trie.h> instead of <marisa.h> if you don't need the
// above I/O interfaces and don't want to include the above I/O headers.
#include "marisa/trie.h"  // IWYU pragma: export

#endif  // MARISA_H_
