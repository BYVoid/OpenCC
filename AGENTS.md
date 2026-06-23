# OpenCC Project Overview

This document compiles the Open Chinese Convert (OpenCC) project information to help quickly familiarize with the code structure, data organization, and accompanying tools.

## Project Overview
- OpenCC is an open-source Chinese Simplified-Traditional and regional variant conversion tool, supporting Simplified↔Traditional, Hong Kong/Macau/Taiwan regional differences, Japanese Shinjitai/Kyujitai character forms, and other conversion schemes.
- The project provides a C++ core library, C language interface, command-line tools, as well as Python, Node.js and other language bindings. The dictionary and program are decoupled for easy customization and extension.
- Main dependencies: `rapidjson` for configuration parsing, `marisa-trie` for high-performance dictionaries (`.ocd2`), optional `Darts` for legacy `.ocd` support, and optional `cppjieba` resources for the experimental `opencc-jieba` segmentation plugin.

## Data and Configuration
- Dictionaries are maintained in `data/dictionary/*.txt`, covering phrases, characters, regional differences, Japanese new characters, and other topic files; converted to `.ocd2` during build for acceleration.
- Default configurations are located in `data/config/`, such as `s2t.json`, `t2s.json`, `s2tw.json`, etc., defining segmenter types, dictionaries used, and combination methods.
- Jieba-backed plugin configurations live under `plugins/jieba/data/config/`, such as `s2twp_jieba.json`; they are packaged only when the optional `opencc-jieba` plugin is built or distributed.
- `data/scheme` and `data/scripts` provide dictionary compilation scripts and specification validation tools.

### Dictionary Placement Rules
- Put regional vocabulary differences, such as Mainland-to-Taiwan term choices and place-name translations, in `TWPhrases.txt` / `TWPhrasesRev.txt`, not in `TWVariantsPhrases.txt`. Example: US state or territory names like `特拉華 -> 德拉瓦`, `新澤西 -> 紐澤西`, and `美屬維爾京羣島 -> 美屬維京群島` belong in `TWPhrases.txt`.
- Reserve `TWVariantsPhrases.txt` for phrase-level exceptions to Taiwan variant character conversion, especially when a phrase must override character-level mappings from `TWVariants.txt`. It is part of `s2tw`, `s2twp`, and `t2tw`; using it for vocabulary translations will incorrectly affect `s2tw`.
- For `s2twp` and `s2hkp`, remember the conversion chain is `STPhrases/STCharacters -> regional phrases -> regional variants`. The generated `STPhrases_GeneratedFromRegionalPhrases` dictionary derives Simplified-to-regional-key entries from the first column of `TWPhrases.txt` and `HKPhrases.txt`; standard Simplified-to-Traditional configs use it in both mmseg segmentation and the first ST conversion stage so whole regional keys do not get stranded in Simplified form. Add manual `STPhrases.txt` entries when the generated mapping is not the desired plain `s2t` output, or when `t2s(regional phrase key)` does not match the real Simplified input.
- Keep `TWPhrases.txt` and `TWPhrasesRev.txt` bidirectionally consistent. Run `bazel test //data/dictionary:dictionary_TWPhrases_reverse_mapping_test` after editing either file.
- For intentional one-way Taiwan vocabulary conversions, keep the forward mapping in `TWPhrases.txt` and add a self-mapping candidate on both sides so `tw2sp` can preserve the Taiwan term. For example, to make `s2twp` convert `信道 -> 通道` without forcing `tw2sp` to convert every `通道` back to `信道`, use `信道 -> 通道` plus `通道 -> 通道` in `TWPhrases.txt`, and make `TWPhrasesRev.txt` map `通道 -> 通道 信道`. The self-mapping keeps the reverse dictionary structurally consistent while making the reverse conversion prefer unchanged `通道`.
- Use `python3 data/scripts/sort.py <file> <file>` for edited dictionary files instead of hand-sorting. Dictionary tests enforce sorted unique keys.
- Add tests to `test/testcases/testcases.json` against the config that actually uses the dictionary. Taiwan vocabulary in `TWPhrases.txt` should normally be tested with `s2twp` / `tw2sp`, not `s2tw`.

### Dictionary Binary Formats: `.ocd` and `.ocd2`
- `.ocd` (legacy format) has `OPENCCDARTS1` as the file header, with the main body being serialized Darts double-array trie data, combined with `BinaryDict` structure to store key-value offsets and concatenation buffers. Loading process is detailed in `src/DartsDict.cpp` and `src/BinaryDict.cpp`. Commonly used in environments requiring `ENABLE_DARTS` for compatibility.
- `.ocd2` (default format) has `OPENCC_MARISA_0.2.5` as the file header, followed by `marisa::Trie` data, then uses the `SerializedValues` module to store all candidate value lists. See `src/MarisaDict.cpp`, `src/SerializedValues.cpp` for details. This format is smaller and loads faster (e.g., `NEWS.md` records `STPhrases` reduced from 4.3MB to 924KB).
- The command-line tool `opencc_dict` supports `text ↔ ocd2` (and optionally `ocd`) conversion. When adding or adjusting dictionaries, first edit `.txt`, then run the tool to generate the target format.

## Development and Testing
- The top-level build system supports CMake, Bazel, Node.js `binding.gyp`, Python `pyproject.toml`, with cross-platform CI integration.
- `src/*Test.cpp`, `data/config/*Test.cpp`, `plugins/jieba/tests/`, `test/`, and `test/golden/` contain tests covering dictionary matching, conversion chains, configuration validation, plugin segmentation, CLI behavior, and golden conversion outputs.
- Tools `opencc_dict`, `opencc_phrase_extract` (`src/tools/`) help developers convert dictionary formats and extract phrases.
- Node.js tests live in `node/test.js`; npm prebuild packaging uses `npm run prebuild` and related scripts in `scripts/`.

## Ecosystem Bindings
- Python module is located in `python/`, providing the `OpenCC` class through the C API.
- Node.js extension is in the `node/` directory, using N-API/Node-API to call the core library. The optional `opencc-jieba` npm package lives in `plugins/jieba/node/` and supplies plugin configs, dictionaries, and platform-specific plugin binaries.
- README lists third-party Swift, Java, Go, WebAssembly and other porting projects, showcasing ecosystem breadth.

## Optional Plugin and Release Packaging
- `BUILD_OPENCC_JIEBA_PLUGIN=ON` enables the C++ jieba plugin in CMake builds; default builds do not require it.
- `plugins/README.md` documents plugin loading, ABI expectations, and standalone plugin builds.
- `scripts/release-windows-winget.ps1` is the Windows portable/WinGet release path and produces the CLI zip, checksum, and WinGet manifests.
- npm release packaging is separate from the native CLI release: `opencc` and `opencc-jieba` are packed as npm `.tgz` artifacts and should be install-tested together when plugin-backed npm configs are changed.

## Common Customization Steps
1. Edit or add dictionary entries in `data/dictionary/*.txt`.
2. Use `opencc_dict` to convert to `.ocd2`.
3. Copy/modify configuration JSON in `data/config` and specify new dictionary files.
4. Add or update test cases in `test/testcases/testcases.json` for normal configs, or `plugins/jieba/tests/data/jieba_comparison_testcases.json` / golden fixtures for jieba-backed behavior.
5. Load custom configuration through `SimpleConverter`, command-line tools, or language bindings to verify results.

> For deeper understanding, read the module documentation in `src/README.md`, or refer to test cases in `test/` to understand conversion chain combinations.

### Common Deviations in Third-Party Implementations (Speculation)
- **Missing segmentation and conversion chain order**: If `group` configuration or dictionary priority is not restored, compound words may be split apart or overwritten by single characters.
- **Missing longest prefix logic**: Character-by-character replacement alone will miss idioms and multi-character word results.
- **Improper UTF-8 handling**: Overlooking multi-byte characters or surrogate pair handling can easily cause offset or truncation issues.
- **Incomplete dictionaries/configuration**: Missing segmentation dictionaries, regional differences and other `.ocd2` files will result in missing words in output.
- **Path and loading process differences**: If OpenCC's path search and configuration parsing details are not followed, the actual loaded resources will differ from official ones, naturally leading to different results.

## Communication Language

Respond in Traditional Chinese (繁體中文) preferred; Simplified Chinese acceptable. When quoting dictionary keys, code identifiers, file names, or any string literal that appears in the codebase in Simplified Chinese, preserve the original Simplified form verbatim — do not transliterate it.

## Commit Message Style

- **First line**: action verb + concise description, no conventional-commit prefix (`feat:`, `fix:`, `perf:`, etc.). Example: `Implement single-dictionary lookup fast-path for PrefixMatch`
- **Body**: one or two sentences summarising motivation and scope, separated from the title by a blank line.
- **Multi-area changes**: add a `Detailed Changes:` section with bold headers and sub-bullets:
  ```
  Detailed Changes:
  - **Section Name**:
    - Sub-point one.
    - Sub-point two.
  ```
- **No** `Co-Authored-By` trailer lines.

## Further Reading

### Contribution Guide
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Complete guide on how to contribute dictionary entries to OpenCC, write test cases, and execute testing procedures.

### Project Documents
- **[src/README.md](src/README.md)** - Detailed technical documentation for core modules.
- **[README.md](README.md)** - Project overview, installation and usage guide.
