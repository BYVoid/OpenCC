#!/usr/bin/env bash

set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <output.dll>" >&2
  exit 2
fi

output="$1"
zig="${ZIG:-zig}"

export ZIG_LOCAL_CACHE_DIR="${ZIG_LOCAL_CACHE_DIR:-$PWD/.zig-local-cache}"
export ZIG_GLOBAL_CACHE_DIR="${ZIG_GLOBAL_CACHE_DIR:-$PWD/.zig-global-cache}"
mkdir -p "$ZIG_LOCAL_CACHE_DIR" "$ZIG_GLOBAL_CACHE_DIR"

zig_log="$PWD/zig-jieba-build.log"

sources=(
  plugins/jieba/src/JiebaSegmentation.cpp
  plugins/jieba/src/JiebaSegmentationPlugin.cpp
  src/Dict.cpp
  src/DictEntry.cpp
  src/Lexicon.cpp
  src/MarisaDict.cpp
  src/Segmentation.cpp
  src/SerializableDict.cpp
  src/SerializedValues.cpp
  src/UTF8Util.cpp
  deps/marisa-0.3.1/lib/marisa/agent.cc
  deps/marisa-0.3.1/lib/marisa/grimoire/io/mapper.cc
  deps/marisa-0.3.1/lib/marisa/grimoire/io/reader.cc
  deps/marisa-0.3.1/lib/marisa/grimoire/io/writer.cc
  deps/marisa-0.3.1/lib/marisa/grimoire/trie/louds-trie.cc
  deps/marisa-0.3.1/lib/marisa/grimoire/trie/tail.cc
  deps/marisa-0.3.1/lib/marisa/grimoire/vector/bit-vector.cc
  deps/marisa-0.3.1/lib/marisa/keyset.cc
  deps/marisa-0.3.1/lib/marisa/trie.cc
)

set +e
"$zig" c++ \
  -target x86_64-windows-gnu \
  -shared \
  -O2 \
  -std=c++17 \
  -w \
  -DNDEBUG \
  -DOPENCC_PLUGIN_BUILD \
  -DOpencc_BUILT_AS_STATIC \
  -I. \
  -Isrc \
  -Iplugins/jieba/include \
  -Iplugins/jieba/deps/cppjieba/include \
  -Iplugins/jieba/deps/cppjieba/deps/limonp/include \
  -Ideps/marisa-0.3.1/include \
  -Ideps/marisa-0.3.1/lib \
  "${sources[@]}" \
  -Wl,--strip-all \
  -Wl,--build-id=sha1 \
  -o "$output" \
  2>"$zig_log"
zig_status=$?
set -e

if [[ $zig_status -ne 0 ]]; then
  cat "$zig_log" >&2
  exit "$zig_status"
fi
