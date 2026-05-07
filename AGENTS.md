# OpenCC Project Overview

This document compiles the Open Chinese Convert (OpenCC) project information to help quickly familiarize with the code structure, data organization, and accompanying tools.

## Project Overview
- OpenCC is an open-source Chinese Simplified-Traditional and regional variant conversion tool, supporting Simplified↔Traditional, Hong Kong/Macau/Taiwan regional differences, Japanese Shinjitai/Kyujitai character forms, and other conversion schemes.
- The project provides a C++ core library, C language interface, command-line tools, as well as Python, Node.js and other language bindings. The dictionary and program are decoupled for easy customization and extension.
- Main dependencies: `rapidjson` for configuration parsing, `marisa-trie` for high-performance dictionaries (`.ocd2`), optional `Darts` for legacy `.ocd` support.

## Data and Configuration
- Dictionaries are maintained in `data/dictionary/*.txt`, covering phrases, characters, regional differences, Japanese new characters, and other topic files; converted to `.ocd2` during build for acceleration.
- Default configurations are located in `data/config/`, such as `s2t.json`, `t2s.json`, `s2tw.json`, etc., defining segmenter types, dictionaries used, and combination methods.
- `data/scheme` and `data/scripts` provide dictionary compilation scripts and specification validation tools.

### Dictionary Binary Formats: `.ocd` and `.ocd2`
- `.ocd` (legacy format) has `OPENCCDARTS1` as the file header, with the main body being serialized Darts double-array trie data, combined with `BinaryDict` structure to store key-value offsets and concatenation buffers. Loading process is detailed in `src/DartsDict.cpp` and `src/BinaryDict.cpp`. Commonly used in environments requiring `ENABLE_DARTS` for compatibility.
- `.ocd2` (default format) has `OPENCC_MARISA_0.2.5` as the file header, followed by `marisa::Trie` data, then uses the `SerializedValues` module to store all candidate value lists. See `src/MarisaDict.cpp`, `src/SerializedValues.cpp` for details. This format is smaller and loads faster (e.g., `NEWS.md` records `STPhrases` reduced from 4.3MB to 924KB).
- The command-line tool `opencc_dict` supports `text ↔ ocd2` (and optionally `ocd`) conversion. When adding or adjusting dictionaries, first edit `.txt`, then run the tool to generate the target format.

## Development and Testing
- The top-level build system supports CMake, Bazel, Node.js `binding.gyp`, Python `pyproject.toml`, with cross-platform CI integration.
- `src/*Test.cpp`, `test/` directories contain Google Test-style unit tests covering dictionary matching, conversion chains, segmentation, and other key logic.
- Tools `opencc_dict`, `opencc_phrase_extract` (`src/tools/`) help developers convert dictionary formats and extract phrases.

## Ecosystem Bindings
- Python module is located in `python/`, providing the `OpenCC` class through the C API.
- Node.js extension is in the `node/` directory, using N-API/Node-API to call the core library.
- README lists third-party Swift, Java, Go, WebAssembly and other porting projects, showcasing ecosystem breadth.

## Common Customization Steps
1. Edit or add dictionary entries in `data/dictionary/*.txt`.
2. Use `opencc_dict` to convert to `.ocd2`.
3. Copy/modify configuration JSON in `data/config` and specify new dictionary files.
4. Load custom configuration through `SimpleConverter`, command-line tools, or language bindings to verify results.

> For deeper understanding, read the module documentation in `src/README.md`, or refer to test cases in `test/` to understand conversion chain combinations.

### Common Deviations in Third-Party Implementations (Speculation)
- **Missing segmentation and conversion chain order**: If `group` configuration or dictionary priority is not restored, compound words may be split apart or overwritten by single characters.
- **Missing longest prefix logic**: Character-by-character replacement alone will miss idioms and multi-character word results.
- **Improper UTF-8 handling**: Overlooking multi-byte characters or surrogate pair handling can easily cause offset or truncation issues.
- **Incomplete dictionaries/configuration**: Missing segmentation dictionaries, regional differences and other `.ocd2` files will result in missing words in output.
- **Path and loading process differences**: If OpenCC's path search and configuration parsing details are not followed, the actual loaded resources will differ from official ones, naturally leading to different results.

## Further Reading

### Contribution Guide
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Complete guide on how to contribute dictionary entries to OpenCC, write test cases, and execute testing procedures.

### Project Documents
- **[src/README.md](src/README.md)** - Detailed technical documentation for core modules.
- **[README.md](README.md)** - Project overview, installation and usage guide.
