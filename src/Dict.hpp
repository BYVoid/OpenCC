/*
 * Open Chinese Convert
 *
 * Copyright 2010-2013 BYVoid <byvoid@byvoid.com>
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
#include "DictEntry.hpp"

namespace opencc {
  class OPENCC_EXPORT Dict {
    public:
      virtual size_t KeyMaxLength() const = 0;
      virtual Optional<DictEntry> Match(const char* word) = 0;
      virtual Optional<DictEntry> MatchPrefix(const char* word);
      virtual vector<DictEntry> MatchAllPrefixes(const char* word);
      Optional<DictEntry> Match(const string& word) {
        return Match(word.c_str());
      }
      Optional<DictEntry> MatchPrefix(const string& word) {
        return MatchPrefix(word.c_str());
      }
      vector<DictEntry> MatchAllPrefixes(const string& word) {
        return MatchAllPrefixes(word.c_str());
      }
      virtual vector<DictEntry> GetLexicon() = 0;
      virtual void LoadFromDict(Dict* dictionary) = 0;
  };
}
