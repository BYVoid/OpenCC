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

#include "DictGroup.hpp"
#include "TextDict.hpp"

using namespace opencc;

DictGroup::DictGroup(const list<DictPtr>& _dicts) :
  keyMaxLength(0), dicts(_dicts) {}

DictGroup::~DictGroup() {}

size_t DictGroup::KeyMaxLength() const {
  return keyMaxLength;
}

Optional<const DictEntry*> DictGroup::Match(const char* word) const {
  for (const auto& dict : dicts) {
    const Optional<const DictEntry*>& prefix = dict->Match(word);
    if (!prefix.IsNull()) {
      return prefix;
    }
  }
  return Optional<const DictEntry*>();
}

Optional<const DictEntry*> DictGroup::MatchPrefix(const char* word) const {
  for (const auto& dict : dicts) {
    const Optional<const DictEntry*>& prefix = dict->MatchPrefix(word);
    if (!prefix.IsNull()) {
      return prefix;
    }
  }
  return Optional<const DictEntry*>();
}

vector<const DictEntry*> DictGroup::MatchAllPrefixes(const char* word) const {
  std::map<size_t, const DictEntry*> matched;
  // Match all prefixes from all dictionaries
  for (const auto& dict : dicts) {
    const vector<const DictEntry*>& entries = dict->MatchAllPrefixes(word);
    for (const auto& entry : entries) {
      size_t len = entry->Key().length();
      // If the current length has already result, skip
      if (matched.find(len) == matched.end()) {
        matched[len] = entry;
      }
    }
  }
  vector<const DictEntry*> matchedEntries;
  for (auto i = matched.rbegin(); i != matched.rend(); i++) {
    matchedEntries.push_back(i->second);
  }
  return matchedEntries;
}

vector<DictEntry> DictGroup::GetLexicon() const {
  vector<DictEntry> allLexicon;
  for (const auto& dict : dicts) {
    auto lexicon = dict->GetLexicon();
    std::copy(lexicon.begin(), lexicon.end(), std::back_inserter(allLexicon));
  }
  std::sort(allLexicon.begin(), allLexicon.end());
  return allLexicon;
}

DictGroupPtr DictGroup::NewFromDict(const Dict& dict) {
  TextDictPtr newDict = TextDict::NewFromDict(dict);
  return DictGroupPtr(new DictGroup(list<DictPtr>{ newDict }));
}
