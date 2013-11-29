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

void DictGroup::AddDict(shared_ptr<Dict> dict) {
  dicts.push_back(dict);
  keyMaxLength = std::max(dict->KeyMaxLength(), keyMaxLength);
}

size_t DictGroup::KeyMaxLength() const {
  return keyMaxLength;
}

Optional<shared_ptr<DictEntry>> DictGroup::MatchPrefix(const char* word) {
  for (auto dict : dicts) {
    Optional<shared_ptr<DictEntry>> prefix = dict->MatchPrefix(word);
    if (!prefix.IsNull()) {
      return prefix;
    }
  }
  return Optional<shared_ptr<DictEntry>>();
}

shared_ptr<vector<shared_ptr<DictEntry>>> DictGroup::MatchAllPrefixes(const char* word) {
  map<size_t, shared_ptr<DictEntry>> matched;
  for (auto dict : dicts) {
    auto entries = dict->MatchAllPrefixes(word);
    for (shared_ptr<DictEntry> entry : *entries) {
      size_t len = entry->key.length();
      if (matched.find(len) == matched.end()) {
        matched[len] = entry;
      }
    }
  }
  shared_ptr<vector<shared_ptr<DictEntry>>> matchedEntries(new vector<shared_ptr<DictEntry>>);
  for (auto entry : matched) {
    matchedEntries->push_back(entry.second);
  }
  std::reverse(matchedEntries->begin(), matchedEntries->end());
  return matchedEntries;
}
