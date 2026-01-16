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

#include "Common.hpp"
#include "DictEntry.hpp"

namespace opencc {
/**
 * Storage of all entries
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT Lexicon {
public:
  Lexicon() {}
  Lexicon(std::vector<std::unique_ptr<DictEntry>> entries_)
      : entries(std::move(entries_)) {}
  Lexicon(const Lexicon&) = delete;
  Lexicon& operator=(const Lexicon&) = delete;

  // Lexicon will take the ownership of the entry.
  void Add(DictEntry* entry) { entries.emplace_back(entry); }

  void Add(std::unique_ptr<DictEntry> entry) {
    entries.push_back(std::move(entry));
  }

  void Sort();

  // Returns true if the lexicon is sorted by key.
  bool IsSorted();

  // Returns true if every key unique (after sorted).
  // When dupkey is set, it is set to the duplicate key.
  bool IsUnique(std::string* dupkey = nullptr);

  const DictEntry* At(size_t index) const { return entries.at(index).get(); }

  size_t Length() const { return entries.size(); }

  std::vector<std::unique_ptr<DictEntry>>::const_iterator begin() const {
    return entries.begin();
  }

  std::vector<std::unique_ptr<DictEntry>>::const_iterator end() const {
    return entries.end();
  }

  static LexiconPtr ParseLexiconFromFile(FILE* fp);

private:
  std::vector<std::unique_ptr<DictEntry>> entries;
};
} // namespace opencc
