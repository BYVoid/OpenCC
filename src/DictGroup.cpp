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

using namespace Opencc;

DictGroup::DictGroup() {
  keyMaxLength = 0;
}

DictGroup::~DictGroup() {
}

void DictGroup::AddDict(Dict* dict) {
  dicts.push_back(dict);
  keyMaxLength = std::max(dict->KeyMaxLength(), keyMaxLength);
}

size_t DictGroup::KeyMaxLength() const {
  return keyMaxLength;
}

Optional<DictEntry> DictGroup::MatchPrefix(const char* word) {
  for (Dict* dict : dicts) {
    Optional<DictEntry> prefix = dict->MatchPrefix(word);
    if (!prefix.IsNull()) {
      return prefix;
    }
  }
  return Optional<DictEntry>();
}

vector<DictEntry> DictGroup::MatchAllPrefixes(const char* word) {
  map<size_t, DictEntry> matched;
  for (Dict* dict : dicts) {
    vector<DictEntry> entries = dict->MatchAllPrefixes(word);
    for (DictEntry entry : entries) {
      size_t len = entry.key.length();
      if (matched.find(len) != matched.end()) {
        matched[len] = entry;
      }
    }
  }
  vector<DictEntry> matchedEntries;
  for (auto entry : matched) {
    matchedEntries.push_back(entry.second);
  }
  return matchedEntries;
}
