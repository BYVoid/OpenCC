#!/usr/bin/env bash

set -euo pipefail

if [[ $# -ne 4 ]]; then
  echo "Usage: $0 <output.node> <node_api.h> <napi.h> <node.lib>" >&2
  exit 2
fi

output="$1"
node_api_header="$2"
napi_header="$3"
node_lib="$4"

zig="${ZIG:-zig}"
node_include_dir="$(dirname "$node_api_header")"
napi_include_dir="$(dirname "$napi_header")"

export ZIG_LOCAL_CACHE_DIR="${ZIG_LOCAL_CACHE_DIR:-$PWD/.zig-local-cache}"
export ZIG_GLOBAL_CACHE_DIR="${ZIG_GLOBAL_CACHE_DIR:-$PWD/.zig-global-cache}"
mkdir -p "$ZIG_LOCAL_CACHE_DIR" "$ZIG_GLOBAL_CACHE_DIR"

node_import_lib="$PWD/node.lib"
cp "$node_lib" "$node_import_lib"
zig_log="$PWD/zig-build.log"

sources=(
  node/opencc.cc
  src/BinaryDict.cpp
  src/Config.cpp
  src/Conversion.cpp
  src/ConversionChain.cpp
  src/Converter.cpp
  src/Dict.cpp
  src/DictConverter.cpp
  src/DictEntry.cpp
  src/DictGroup.cpp
  src/Lexicon.cpp
  src/MarisaDict.cpp
  src/MaxMatchSegmentation.cpp
  src/PluginSegmentation.cpp
  src/PrefixMatch.cpp
  src/Segmentation.cpp
  src/SerializedValues.cpp
  src/TextDict.cpp
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
  -std=c++20 \
  -w \
  -Wno-nullability-completeness \
  -DNDEBUG \
  '-DVERSION="1.3.1"' \
  -DNAPI_DISABLE_CPP_EXCEPTIONS \
  -DOpencc_BUILT_AS_STATIC \
  -I. \
  -Inode \
  -Isrc \
  -Ideps/rapidjson-1.1.0 \
  -Ideps/marisa-0.3.1/include \
  -Ideps/marisa-0.3.1/lib \
  -I"$node_include_dir" \
  -I"$napi_include_dir" \
  "${sources[@]}" \
  "$node_import_lib" \
  -Wl,--build-id=sha1 \
  -o "$output" \
  2>"$zig_log"
zig_status=$?
set -e

if [[ $zig_status -ne 0 ]]; then
  cat "$zig_log" >&2
  exit "$zig_status"
fi
