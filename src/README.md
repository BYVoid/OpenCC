# Source Code

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

## Main Files


## CPP Files

- Lexicon: Storage of all entries
- BinaryDict: Binary dictionary for faster deserialization
- CmdLineOutput: Output command line usage
- Common: Common definitions
- Config: Configuration loader
- Conversation: Conversion interface
- Converter: Controller of segmentation and conversion
- DartsDict: Darts dictionary
- Dict: Abstract class of dictionary
- DictConverter: Converts a dictionary from a format to another.
- DictEntry: Key-values pair entry
- DictGroup: Group of dictionaries
- Exception: Exceptions
- MarisaDict: Marisa dictionary.
- MaxMatchSegmentation: Implementation of maximal match segmentation
- Segmentation: Abstract segmentation
- Segments: Segmented text
- SerializableDict: Serializable dictionary interface
- SerializedValues: Binary format for dictionary values serialization.
- SimpleConverter: A high level converter
- TextDict: Text dictionary
- UTF8Util: UTF8 std::string utilities
