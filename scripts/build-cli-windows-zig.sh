#!/usr/bin/env bash
#
# Cross-compile the OpenCC command-line executable for Windows x64 using Zig.
#
# This script is intentionally a direct compiler invocation instead of a full
# release build.  It is useful as a fast smoke test for Windows-facing C++ code
# paths when developing on Unix-like hosts:
#
#   - _WIN32 preprocessor branches are compiled.
#   - MinGW-style Windows headers are parsed.
#   - Windows import libraries such as shell32 are linked.
#   - The CLI entry wrapper, UTF-8 argv handling, and UTF-8 file I/O boundary
#     are checked without requiring a Windows machine.
#
# It does not run the generated executable.  Use real Windows CI or a Windows
# VM for runtime validation of command-line parsing, pipes, and Unicode paths.
#
# Usage:
#   ./scripts/build-cli-windows-zig.sh [output.exe]
#
# Environment:
#   ZIG                 Zig executable to use. Defaults to "zig".
#   ZIG_TARGET          Zig target triple. Defaults to "x86_64-windows-gnu".
#   ZIG_OPTIMIZE        Optimization flag. Defaults to "-O2".
#   OPENCC_VERSION      VERSION macro value. Defaults to MODULE.bazel version.
#   ZIG_LOCAL_CACHE_DIR Zig local cache. Defaults to "$ROOT/.zig-local-cache".
#   ZIG_GLOBAL_CACHE_DIR Zig global cache. Defaults to "$ROOT/.zig-global-cache".

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

zig="${ZIG:-zig}"
target="${ZIG_TARGET:-x86_64-windows-gnu}"
optimize="${ZIG_OPTIMIZE:--O2}"
output="${1:-build/zig-windows/opencc.exe}"

if [[ $# -gt 1 ]]; then
  echo "Usage: $0 [output.exe]" >&2
  exit 2
fi

version="${OPENCC_VERSION:-}"
if [[ -z "$version" ]]; then
  version="$(
    sed -n 's/^[[:space:]]*version[[:space:]]*=[[:space:]]*"\([^"]*\)".*/\1/p' \
      MODULE.bazel | head -n 1
  )"
fi
version="${version:-1.3.2}"

export ZIG_LOCAL_CACHE_DIR="${ZIG_LOCAL_CACHE_DIR:-$ROOT_DIR/.zig-local-cache}"
export ZIG_GLOBAL_CACHE_DIR="${ZIG_GLOBAL_CACHE_DIR:-$ROOT_DIR/.zig-global-cache}"
mkdir -p "$ZIG_LOCAL_CACHE_DIR" "$ZIG_GLOBAL_CACHE_DIR" "$(dirname "$output")"

zig_log="${output}.zig.log"

# Keep this source list explicit.  The goal is to catch Windows-specific
# compile/link problems in the CLI and core library, not to hide missing source
# additions behind a separate build generator.
sources=(
  src/tools/CommandLine.cpp
  src/tools/CommandLineMain.cpp
  src/tools/PlatformIO.cpp
  src/BinaryDict.cpp
  src/Config.cpp
  src/Conversion.cpp
  src/ConversionChain.cpp
  src/Converter.cpp
  src/DartsDict.cpp
  src/Dict.cpp
  src/DictConverter.cpp
  src/DictEntry.cpp
  src/DictGroup.cpp
  src/Lexicon.cpp
  src/MarisaDict.cpp
  src/MaxMatchSegmentation.cpp
  src/PluginSegmentation.cpp
  src/PhraseExtract.cpp
  src/PrefixMatch.cpp
  src/Segmentation.cpp
  src/SerializableDict.cpp
  src/SerializedValues.cpp
  src/SimpleConverter.cpp
  src/TextDict.cpp
  src/UTF8StringSlice.cpp
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
  -target "$target" \
  "$optimize" \
  -std=c++17 \
  -w \
  -Wno-nullability-completeness \
  -DNDEBUG \
  "-DVERSION=\"$version\"" \
  -DOpencc_BUILT_AS_STATIC \
  -I. \
  -Isrc \
  -Ideps/rapidjson-1.1.0 \
  -Ideps/marisa-0.3.1/include \
  -Ideps/marisa-0.3.1/lib \
  -Ideps/tclap-1.2.5 \
  -Ideps/darts-clone-0.32 \
  "${sources[@]}" \
  -lshell32 \
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

if [[ -s "$zig_log" ]]; then
  echo "Zig completed with warnings; see $zig_log" >&2
else
  rm -f "$zig_log"
fi

ls -lh "$output"
