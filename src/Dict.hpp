/*
 * Open Chinese Convert
 *
 * Copyright 2010-2020 Carbo Kuo <byvoid@byvoid.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <list>
#include <string_view>

#include "Common.hpp"
#include "DictEntry.hpp"

namespace opencc {

/**
 * Result of a PrefixMatch fast-path lookup.
 * key and value are non-owning views; see PrefixMatch::MatchPrefixView()
 * for lifetime details.
 */
struct OPENCC_EXPORT PrefixMatchView {
  bool matched = false;
  size_t keyLength = 0;
  std::string_view key;
  std::string_view value;
};

/**
 * Abstract class of dictionary
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT Dict {
public:
  /**
   * Matches a word exactly and returns the DictEntry or Optional::Null().
   */
  virtual Optional<const DictEntry*> Match(const char* word,
                                           size_t len) const = 0;

  /**
   * Matches a word exactly and returns the DictEntry or Optional::Null().
   */
  Optional<const DictEntry*> Match(const std::string& word) const {
    return Match(word.c_str(), word.length());
  }

  /**
   * Matches the longest matched prefix of a word.
   * For example given a dictionary having "a", "an", "b", "ba", "ban", "bana",
   * the longest prefix of "banana" matched is "bana".
   */
  virtual Optional<const DictEntry*> MatchPrefix(const char* word,
                                                 size_t len) const;

  /**
   * Matches the longest matched prefix of a word.
   */
  Optional<const DictEntry*> MatchPrefix(const char* word) const {
    return MatchPrefix(word, KeyMaxLength());
  }

  /**
   * Matches the longest matched prefix of a word.
   */
  Optional<const DictEntry*> MatchPrefix(const std::string& word) const {
    return MatchPrefix(word.c_str(), word.length());
  }

  /**
   * Returns all matched prefixes of a word, sorted by the length (desc).
   * For example given a dictionary having "a", "an", "b", "ba", "ban", "bana",
   * all the matched prefixes of "banana" are "bana", "ban", "ba", "b".
   */
  virtual std::vector<const DictEntry*> MatchAllPrefixes(const char* word,
                                                         size_t len) const;

  /**
   * Returns all matched prefixes of a word, sorted by the length (desc).
   */
  std::vector<const DictEntry*>
  MatchAllPrefixes(const std::string& word) const {
    return MatchAllPrefixes(word.c_str(), word.length());
  }

  /**
   * Returns the length of the longest key in the dictionary.
   */
  virtual size_t KeyMaxLength() const = 0;

  /**
   * Returns all entries in the dictionary.
   */
  virtual LexiconPtr GetLexicon() const = 0;

  /**
   * Returns child dictionaries when this dictionary is a group.
   */
  virtual const std::list<DictPtr>* GetDictGroupItems() const {
    return nullptr;
  }

  /**
   * Returns true if this dict can handle prefix queries directly without
   * PrefixMatch building a lookup table. Subclasses that override
   * MatchPrefixValue() must also override this to return true.
   */
  virtual bool SupportsFastPrefixMatch() const { return false; }

  /**
   * Fast-path prefix match. Only called when SupportsFastPrefixMatch() is
   * true. Returns a PrefixMatchView with matched=false on no match.
   * See PrefixMatch::MatchPrefixView() for key/value lifetime details.
   */
  virtual PrefixMatchView MatchPrefixValue(const char* word,
                                           size_t len) const {
    return PrefixMatchView{};
  }

  virtual ~Dict() = default;
};
} // namespace opencc
