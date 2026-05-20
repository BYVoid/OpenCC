# OpenCC Source Layout

This directory contains the C++ core library, the public C API, command-line
tools, and native extension entry points. The code is organized around a simple
conversion pipeline:

1. Load a JSON configuration.
2. Load segmenters and dictionaries referenced by that configuration.
3. Segment input text.
4. Apply one or more dictionary-based conversion stages.
5. Return converted text or inspection data.

The data files that drive this pipeline live outside this directory:

- `data/config/*.json`: conversion schemes.
- `data/dictionary/*.txt`: source dictionaries.
- generated `.ocd2` files: marisa-trie dictionary binaries.

## Main Pipeline

### Configuration

- `Config.hpp`, `Config.cpp`
  - Finds and parses JSON configuration files.
  - Builds segmenters, dictionaries, dictionary groups, conversions, and
    converters.
  - Maintains dictionary search paths from explicit `--path` values,
    `argv[0]`, installed package data paths, and Windows portable-layout
    locations.
  - Uses UTF-16-capable file checks on Windows for internal config and
    resource paths.

### Segmentation

- `Segmentation.hpp`, `Segmentation.cpp`
  - Base interface for segmenters.
- `MaxMatchSegmentation.hpp`, `MaxMatchSegmentation.cpp`
  - Maximum forward matching segmenter used by normal OpenCC configs.
  - Uses dictionary prefix matches to keep phrases intact.
- `PluginSegmentation.hpp`, `PluginSegmentation.cpp`
  - Runtime-loaded segmentation plugin support.
  - Used by the Jieba plugin.
- `Segments.hpp`
  - Segment container used between segmenters, conversions, and inspection
    output.

### Conversion

- `Converter.hpp`, `Converter.cpp`
  - Owns a segmenter and a conversion chain.
  - Provides high-level string and buffer conversion.
- `ConversionChain.hpp`, `ConversionChain.cpp`
  - Ordered list of conversion stages.
- `Conversion.hpp`, `Conversion.cpp`
  - One dictionary-backed conversion stage.
- `ConversionInspection.hpp`
  - Data structures returned by inspection mode.

The core conversion path depends on segmentation and longest-prefix dictionary
matching. Character-by-character replacement is not equivalent to OpenCC
behavior because phrase priority and multi-stage conversion order matter.

## Dictionaries

### Interfaces and Shared Types

- `Dict.hpp`, `Dict.cpp`
  - Abstract dictionary interface.
  - Supports exact match, prefix match, all-prefix match, and enumeration.
- `DictEntry.hpp`, `DictEntry.cpp`
  - Key/value entry representation.
- `PrefixMatch.hpp`, `PrefixMatch.cpp`
  - Prefix match result.
- `Lexicon.hpp`, `Lexicon.cpp`
  - In-memory collection of dictionary entries.
- `SerializableDict.hpp`
  - Template helpers for loading serialized dictionaries from files.
- `SerializedValues.hpp`, `SerializedValues.cpp`
  - Compact storage for candidate value lists used by `.ocd2`.

### Implementations

- `TextDict.hpp`, `TextDict.cpp`
  - Tab-delimited text dictionary.
  - Useful for source data and tests.
- `MarisaDict.hpp`, `MarisaDict.cpp`
  - Default `.ocd2` dictionary format.
  - Uses marisa-trie for compact prefix lookup.
- `DartsDict.hpp`, `DartsDict.cpp`
  - Legacy `.ocd` dictionary format.
  - Requires Darts support.
- `BinaryDict.hpp`, `BinaryDict.cpp`
  - Legacy binary payload support used with Darts serialization.
- `DictGroup.hpp`, `DictGroup.cpp`
  - Ordered group of dictionaries.
  - Tries dictionaries in sequence and returns the first usable match.
- `DictConverter.hpp`, `DictConverter.cpp`
  - Converts dictionary files between supported formats.

## Public APIs

### C++ API

- `SimpleConverter.hpp`, `SimpleConverter.cpp`
  - High-level C++ wrapper around `Config` and `Converter`.
  - Accepts a config name/path and optional search paths.
  - Throws C++ exceptions on failures.

### C API

- `opencc.h`
  - Stable C ABI for downstream users and bindings.
  - Exposes `opencc_open`, `opencc_open_w`, `opencc_convert_utf8`,
    `opencc_convert_utf8_free`, `opencc_close`, and error helpers.

Windows path semantics need care:

- `opencc_open_w(const wchar_t*)` is the explicit UTF-16 Windows API.
- `opencc_open(const char*)` keeps the historical Windows/MSVC narrow-string
  behavior. Do not silently change its encoding contract without a migration
  plan.
- New Windows path-taking C APIs should use explicit names such as
  `*_utf8` or `*_w` rather than relying on ambiguous `char*` semantics.

### Python Extension

- `py_opencc.cpp`
  - Python module binding built on top of the C++ core.

## Command-Line Tools

The native CLI tools live under `src/tools`.

- `CommandLine.cpp`
  - Small executable entry point.
  - On Windows/MSVC, obtains command-line arguments through wide Windows APIs
    and converts them to UTF-8 before dispatching to the CLI core.
- `CommandLineMain.hpp`, `CommandLineMain.cpp`
  - Main `opencc` command implementation.
  - Handles conversion, segmentation/inspection modes, measurement output,
    stream conversion, and explicit `--in-place` conversion.
- `PlatformIO.hpp`, `PlatformIO.cpp`
  - CLI platform boundary for path-sensitive operations.
  - Handles UTF-8 file open/remove/replace, temporary output files, same-file
    detection, real path resolution, and Windows argv collection.
- `DictConverter.cpp`
  - `opencc_dict` implementation.
- `PhraseExtract.cpp`
  - `opencc_phrase_extract` implementation.
- `CmdLineOutput.hpp`
  - Shared command-line help formatting.

CLI file conversion must remain streaming. Do not replace stream processing
with "read whole file into memory" logic. In-place conversion is intentionally
opt-in:

- Without `--in-place`, `-i` and `-o` referring to the same actual file is
  rejected.
- With `--in-place`, output is written to a temporary file next to the target,
  then the target is replaced after conversion succeeds.

## Platform and Path Handling

OpenCC uses UTF-8 internally for paths unless an API explicitly documents a
different contract. Windows code should convert to UTF-16 at the platform
boundary.

Important files:

- `UTF8Util.hpp`, `UTF8Util.cpp`
  - UTF-8 helpers and platform string conversion helpers.
- `WinUtil.hpp`
  - Small Windows-only UTF-8/UTF-16 conversion helpers.
- `tools/PlatformIO.*`
  - CLI path boundary.

Maintenance rules:

- Do not add raw `fopen`, `std::fstream`, `stat`, `std::tmpnam`, or narrow
  Win32 path calls to new Windows-sensitive paths.
- Prefer existing platform helpers or add a clearly named boundary helper.
- Keep public API encoding contracts explicit.
- Validate Windows path changes with real Windows CI. Zig cross-compilation is
  useful for compile/link coverage, but it does not execute Windows runtime
  path behavior.

## Plugins

- `plugin/OpenCCPlugin.h`
  - C plugin ABI used by loadable segmentation plugins.
- `PluginSegmentation.*`
  - Host-side plugin loader and adapter.

The Jieba plugin lives outside `src` under `plugins/jieba`.

## Tests

Most core modules have adjacent `*Test.cpp` files in this directory. Important
test groups include:

- `ConfigTest.cpp`
  - Config search paths and Unicode config path handling.
- `SimpleConverterTest.cpp`
  - C++ wrapper behavior and C API basics.
- `Conversion*Test.cpp`
  - Conversion and inspection behavior.
- `MaxMatchSegmentationTest.cpp`
  - Segmenter behavior.
- `MarisaDictTest.cpp`, `TextDictTest.cpp`, `DictGroupTest.cpp`
  - Dictionary implementations.
- `UTF8*Test.cpp`
  - UTF-8 slicing and utility behavior.

CLI tests live in `test/CommandLineConvertTest.cpp` because they execute the
built command-line binary. They cover streaming behavior, Unicode paths,
measurement output, inspection modes, and in-place conversion safety.

## Build Integration

The source tree is built through both CMake and Bazel:

- `src/CMakeLists.txt`
- `src/BUILD.bazel`
- `src/tools/CMakeLists.txt`
- `src/tools/BUILD.bazel`

When adding or renaming source files, update both build systems and any direct
cross-build scripts that carry explicit source lists.
