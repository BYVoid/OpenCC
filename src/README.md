# Source code

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
