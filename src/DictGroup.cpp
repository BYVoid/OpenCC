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

DictGroup::DictGroup() {
  keyMaxLength = 0;
}

DictGroup::~DictGroup() {}

void DictGroup::AddDict(DictPtr dict) {
  dicts.push_back(dict);
  keyMaxLength = std::max(dict->KeyMaxLength(), keyMaxLength);
}

size_t DictGroup::KeyMaxLength() const {
  return keyMaxLength;
}

Optional<DictEntryPtr> DictGroup::Match(const char* word) {
  for (auto dict : dicts) {
    Optional<DictEntryPtr> prefix = dict->Match(word);
    if (!prefix.IsNull()) {
      return prefix;
    }
  }
  return Optional<DictEntryPtr>();
}

Optional<DictEntryPtr> DictGroup::MatchPrefix(const char* word) {
  for (auto dict : dicts) {
    Optional<DictEntryPtr> prefix = dict->MatchPrefix(word);
    if (!prefix.IsNull()) {
      return prefix;
    }
  }
  return Optional<DictEntryPtr>();
}

DictEntryPtrVectorPtr DictGroup::MatchAllPrefixes(const char* word) {
  std::map<size_t, DictEntryPtr> matched;
  for (auto dict : dicts) {
    auto entries = dict->MatchAllPrefixes(word);
    for (DictEntryPtr entry : *entries) {
      size_t len = entry->key.length();
      if (matched.find(len) == matched.end()) {
        matched[len] = entry;
      }
    }
  }
  DictEntryPtrVectorPtr matchedEntries(new DictEntryPtrVector);
  for (auto entry : matched) {
    matchedEntries->push_back(entry.second);
  }
  std::reverse(matchedEntries->begin(), matchedEntries->end());
  return matchedEntries;
}

DictEntryPtrVectorPtr DictGroup::GetLexicon() {
  DictEntryPtrVectorPtr allLexicon(new DictEntryPtrVector);
  for (auto dict : dicts) {
    auto lexicon = dict->GetLexicon();
    std::copy(lexicon->begin(), lexicon->end(), std::back_inserter(*allLexicon));
  }
  std::sort(allLexicon->begin(), allLexicon->end(), DictEntry::PtrCmp);
  return allLexicon;
}

void DictGroup::LoadFromDict(Dict* dictionary) {
  TextDictPtr dict(new TextDict);

  dict->LoadFromDict(dictionary);
  AddDict(dict);
}
