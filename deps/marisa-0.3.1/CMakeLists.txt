include_directories(include)
include_directories(lib)

set(
  LIBMARISA_SOURCES
  lib/marisa/trie.cc
  lib/marisa/agent.cc
  lib/marisa/grimoire/io/reader.cc
  lib/marisa/grimoire/io/writer.cc
  lib/marisa/grimoire/io/mapper.cc
  lib/marisa/grimoire/trie/louds-trie.cc
  lib/marisa/grimoire/trie/tail.cc
  lib/marisa/grimoire/vector/bit-vector.cc
  lib/marisa/keyset.cc
)

add_library(marisa STATIC ${LIBMARISA_SOURCES})
set_target_properties(marisa PROPERTIES POSITION_INDEPENDENT_CODE ON)
source_group(marisa FILES ${LIBMARISA_SOURCES})

# Target Windows 8 for PrefetchVirtualMemory / WIN32_MEMORY_RANGE_ENTRY
if(WIN32)
  target_compile_definitions(marisa PRIVATE _WIN32_WINNT=_WIN32_WINNT_WIN8)
endif()
