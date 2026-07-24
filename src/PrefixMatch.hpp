/*
 * Open Chinese Convert
 *
 * Copyright 2010-2026 Carbo Kuo and contributors
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

#include "Common.hpp"
#include "Dict.hpp"

namespace opencc {

class OPENCC_EXPORT PrefixMatch {
public:
  class Tables;

  struct Match {
    bool matched;
    size_t keyLength;
    const std::string* key;
    const std::string* value;
  };

  explicit PrefixMatch(const DictPtr& dict);
  ~PrefixMatch();

  Match MatchPrefix(const char* word, size_t len) const;

  /**
   * Like MatchPrefix but returns non-owning string_view fields without
   * copying key or value into thread-local storage.
   *
   * Lifetime of the returned views:
   *  - @b key: points into the caller's input buffer (fast-path singleDict,
   *    where both MarisaDict and DartsDict return a slice of @p word) or into
   *    PrefixMatch-owned table storage (table-path LeafMatcher).  Callers
   *    must not assume one or the other; copy if the key needs to outlive the
   *    current input position.
   *  - @b value: valid for the lifetime of the underlying dictionary
   *    (fast-path) or the lifetime of this PrefixMatch's tables (table-path).
   */
  PrefixMatchView MatchPrefixView(const char* word, size_t len) const;

  /**
   * Returns the number of leading bytes of @p word that are guaranteed not
   * to begin any dictionary key: whole UTF-8 characters whose lead byte
   * starts no key (ideographic description operators excluded, so the
   * caller's IDS grouping is preserved). Callers may consume the returned
   * run without calling MatchPrefixView() at any position inside it. The
   * scan is vectorized for ASCII runs. Returns 0 when the next character
   * requires a real lookup.
   */
  size_t SkipUnmatchable(const char* word, size_t len) const;

private:
  static void AppendCacheKey(const DictPtr& dict, std::string* output);
  static void CollectLeafDicts(const DictPtr& dict,
                               std::vector<std::weak_ptr<const Dict>>* output);

  // Always non-null after construction. On the single-dictionary fast path
  // it carries only the skip table; on the table path it also owns the
  // matcher. Kept opaque so this header stays free of the skip-scan and
  // SIMD intrinsic headers.
  std::shared_ptr<const Tables> tables;
  DictPtr singleDict;
};

} // namespace opencc
