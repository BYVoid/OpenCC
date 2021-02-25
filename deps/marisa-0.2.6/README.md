### README

#### Project name

marisa-trie

#### Project summary

MARISA: Matching Algorithm with Recursively Implemented StorAge

#### Latest version

0.2.6

#### Description

Matching Algorithm with Recursively Implemented StorAge (MARISA) is a static and space-efficient trie data structure. And libmarisa is a C++ library to provide an implementation of MARISA. Also, the package of libmarisa contains a set of command line tools for building and operating a MARISA-based dictionary.

A MARISA-based dictionary supports not only lookup but also reverse lookup, common prefix search and predictive search.

* Lookup is to check whether or not a given string exists in a dictionary.
* Reverse lookup is to restore a key from its ID.
* Common prefix search is to find keys from prefixes of a given string.
* Predictive search is to find keys starting with a given string.

The biggest advantage of libmarisa is that its dictionary size is considerably more compact than others. See below for the dictionary size of other implementations.

* Input
  * Source: enwiki-20121101-all-titles-in-ns0.gz
  * Contents: all page titles of English Wikipedia (Nov. 2012)
  * Number of keys: 9,805,576
  * Total size: 200,435,403 bytes (plain) / 54,933,690 bytes (gzipped)

|Implementation|Size (bytes)|Remarks                    |
|:-------------|-----------:|--------------------------:|
|darts-clone   | 376,613,888|Compacted double-array trie|
|tx-trie       | 127,727,058|LOUDS-based trie           |
|marisa-trie   |  50,753,560|MARISA trie                |

#### Documentation

* README (English): https://s-yata.github.io/marisa-trie/docs/readme.en.html
* README (Japanese): https://s-yata.github.io/marisa-trie/docs/readme.ja.html

#### Build instructions

You can get the latest version via `git clone`. Then, you can generate a `configure` script via `autoreconf -i`. After that, you can build and install libmarisa and its command line tools via `configure` and `make`. For details, see also documentation in `docs`.

```
$ git clone https://github.com/s-yata/marisa-trie.git
$ cd marisa-trie
$ autoreconf -i
$ ./configure --enable-native-code
$ make
$ make install
```

#### Source code license

* The BSD 2-clause License
* The LGPL 2.1 or any later version
