/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 Carbo Kuo <byvoid@byvoid.com>
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

#include "Common.hpp"
#include "Dict.hpp"

namespace opencc {

/**
 * Group of dictionaries.
 *
 * DictGroup preserves dictionary order. With the current ShortCircuit match
 * policy, exact lookup returns the first match from the first child dictionary
 * that contains the key.
 *
 * OpenCC's built-in conversion and mmseg segmentation paths do not call
 * DictGroup::MatchPrefix() or DictGroup::MatchAllPrefixes() directly. They
 * construct a PrefixMatch from the group, and PrefixMatch uses
 * GetDictGroupItems() to build its own group-aware prefix matcher. The prefix
 * methods below are still part of the Dict API for direct DictGroup callers
 * and tests.
 *
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT DictGroup : public Dict {
public:
  explicit DictGroup(const std::list<DictPtr>& dicts);

  DictGroup(const std::list<DictPtr>& dicts, DictGroupMatchPolicy matchPolicy);

  static DictGroupPtr NewFromDict(const Dict& dict);

  virtual ~DictGroup();

  /**
   * Returns the maximum KeyMaxLength() among all child dictionaries.
   */
  virtual size_t KeyMaxLength() const;

  /**
   * Matches the key exactly against child dictionaries in group order.
   * Returns the first child dictionary's exact match, if any.
   */
  virtual Optional<const DictEntry*> Match(const char* word, size_t len) const;

  /**
   * Matches the longest prefix within the first child dictionary that has any
   * prefix match. A shorter match from an earlier child dictionary therefore
   * wins over a longer match from a later child dictionary.
   *
   * This method is not used by OpenCC's normal conversion or mmseg
   * segmentation paths; PrefixMatch expands the group via GetDictGroupItems()
   * instead.
   */
  virtual Optional<const DictEntry*> MatchPrefix(const char* word,
                                                 size_t len) const;

  /**
   * Returns prefix matches from all child dictionaries, sorted by key length
   * descending. When multiple child dictionaries have matches of the same key
   * length, the first child dictionary wins.
   *
   * This method is not used by OpenCC's normal conversion or mmseg
   * segmentation paths; PrefixMatch expands the group via GetDictGroupItems()
   * instead.
   */
  virtual std::vector<const DictEntry*> MatchAllPrefixes(const char* word,
                                                         size_t len) const;

  /**
   * Returns a merged lexicon from all child dictionaries.
   *
   * Duplicate keys are not deduplicated here. PrefixMatch handles group
   * priority while building its lookup table.
   */
  virtual LexiconPtr GetLexicon() const;

  /**
   * Exposes child dictionaries to callers that need group-aware behavior.
   * PrefixMatch uses this to preserve nested group boundaries and dictionary
   * order.
   */
  virtual const std::list<DictPtr>* GetDictGroupItems() const {
    return &dicts;
  }

  /**
   * Returns the child dictionaries by value.
   */
  const std::list<DictPtr> GetDicts() const { return dicts; }

  /**
   * Returns how this group resolves matches across child dictionaries.
   */
  virtual DictGroupMatchPolicy GetMatchPolicy() const { return matchPolicy; }

private:
  const size_t keyMaxLength;
  const std::list<DictPtr> dicts;
  const DictGroupMatchPolicy matchPolicy;
};

/**
 * Group of dictionaries with union prefix-match semantics.
 *
 * Exact lookup still uses dictionary order for duplicate keys. Prefix lookup
 * chooses the longest child match, using dictionary order only as a tie-breaker.
 * MatchAllPrefixes() has the same behavior as DictGroup: all matched prefix
 * lengths are returned once, and earlier dictionaries win equal-length ties.
 */
class OPENCC_EXPORT UnionDictGroup : public DictGroup {
public:
  explicit UnionDictGroup(const std::list<DictPtr>& dicts);

  virtual Optional<const DictEntry*> MatchPrefix(const char* word,
                                                 size_t len) const;
};
} // namespace opencc
