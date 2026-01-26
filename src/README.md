# Source code

## Code Modules and Flow
1. **Configuration Loading (`src/Config.cpp`)**
   - Reads JSON configuration (located in `data/config/*.json`), parses segmenter definitions and conversion chains.
   - Loads different dictionary formats (plain text, `ocd2`, dictionary groups) based on the `type` field, with support for additional search paths.
   - Creates `Converter` objects that hold segmenters and conversion chains.

2. **Segmentation (`src/MaxMatchSegmentation.cpp`)**
   - The default segmentation type is `mmseg`, i.e., Maximum Forward Matching.
   - Performs longest prefix matching using the dictionary, splitting input into `Segments`; unmatched UTF-8 fragments are preserved by character length.

3. **Conversion Chain (`src/ConversionChain.cpp`, `src/Conversion.cpp`)**
   - The conversion chain is an ordered list of `Conversion` objects, each node relies on a dictionary to replace segments with target values through longest prefix matching.
   - Supports advanced scenarios like phrase priority, variant character replacement, and multi-stage composition.

4. **Dictionary System**
   - Abstract interface `Dict` unifies prefix matching, all-prefix matching, and dictionary traversal.
   - `TextDict` (`.txt`) builds dictionaries from tab-delimited plain text; `MarisaDict` (`.ocd2`) provides high-performance trie structures; `DictGroup` can compose multiple dictionaries into a sequential collection.
   - `SerializableDict` defines serialization and file loading logic, which command-line tools use to convert between different formats.

5. **API Encapsulation**
   - `SimpleConverter` (high-level C++ interface) encapsulates `Config + Converter`, providing various overloads for string, pointer buffer, and partial length conversion.
   - `opencc.h` exposes the C API: `opencc_open`, `opencc_convert_utf8`, etc., for language bindings and command-line reuse.
   - The command-line program `opencc` (`src/tools/CommandLine.cpp`) demonstrates batch conversion, stream reading, auto-flushing, and same-file input/output handling.

## Dictionary

### Interface

* Dict: Declares `Match` and related functions.
* SerializableDict: Declares dictionary serialization and deserialization functions.

### Implementations

* TextDict: Tabular separated dictionary format.
* BinaryDict: Stores keys and values in binary format. For serialization only.
* DartsDict: Double-array trie (`.ocd`).
* MarisaDict: Marisa trie (`.ocd2`).
* DictGroup: A wrap of a group of dictionaries. Iterates one by one until a match.

## Conversion
