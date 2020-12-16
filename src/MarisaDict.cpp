/*
 * Open Chinese Convert
 *
 * Copyright 2020 Carbo Kuo <byvoid@byvoid.com>
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

#include <algorithm>
#include <cstring>
#include <unordered_map>

#include "marisa.h"

#include "Lexicon.hpp"
#include "MarisaDict.hpp"
#include "SerializedValues.hpp"

using namespace opencc;

namespace {
static const char* OCD2_HEADER = "OPENCC_MARISA_0.2.5";
}

class MarisaDict::MarisaInternal {
public:
  std::unique_ptr<marisa::Trie> marisa;

  MarisaInternal() : marisa(new marisa::Trie()) {}
};

MarisaDict::MarisaDict() : internal(new MarisaInternal()) {}

MarisaDict::~MarisaDict() {}

size_t MarisaDict::KeyMaxLength() const { return maxLength; }

Optional<const DictEntry*> MarisaDict::Match(const char* word,
                                             size_t len) const {
  if (len > maxLength) {
    return Optional<const DictEntry*>::Null();
  }
  const marisa::Trie& trie = *internal->marisa;
  marisa::Agent agent;
  agent.set_query(word, len);
  if (trie.lookup(agent)) {
    return Optional<const DictEntry*>(lexicon->At(agent.key().id()));
  } else {
    return Optional<const DictEntry*>::Null();
  }
}

Optional<const DictEntry*> MarisaDict::MatchPrefix(const char* word,
                                                   size_t len) const {
  const marisa::Trie& trie = *internal->marisa;
  marisa::Agent agent;
  agent.set_query(word, (std::min)(maxLength, len));
  const DictEntry* match = nullptr;
  while (trie.common_prefix_search(agent)) {
    match = lexicon->At(agent.key().id());
  }
  if (match == nullptr) {
    return Optional<const DictEntry*>::Null();
  } else {
    return Optional<const DictEntry*>(match);
  }
}

std::vector<const DictEntry*> MarisaDict::MatchAllPrefixes(const char* word,
                                                           size_t len) const {
  const marisa::Trie& trie = *internal->marisa;
  marisa::Agent agent;
  agent.set_query(word, (std::min)(maxLength, len));
  std::vector<const DictEntry*> matches;
  while (trie.common_prefix_search(agent)) {
    matches.push_back(lexicon->At(agent.key().id()));
  }
  std::reverse(matches.begin(), matches.end());
  return matches;
}

LexiconPtr MarisaDict::GetLexicon() const { return lexicon; }

MarisaDictPtr MarisaDict::NewFromFile(FILE* fp) {
  // Verify file header
  size_t headerLen = strlen(OCD2_HEADER);
  void* buffer = malloc(sizeof(char) * headerLen);
  size_t bytesRead = fread(buffer, sizeof(char), headerLen, fp);
  if (bytesRead != headerLen || memcmp(buffer, OCD2_HEADER, headerLen) != 0) {
    throw InvalidFormat("Invalid OpenCC dictionary header");
  }
  free(buffer);
  // Read Marisa Trie
  MarisaDictPtr dict(new MarisaDict());
  marisa::fread(fp, dict->internal->marisa.get());
  std::shared_ptr<SerializedValues> serialized_values =
      SerializedValues::NewFromFile(fp);
  LexiconPtr values_lexicon = serialized_values->GetLexicon();
  // Extract lexicon from built Marisa Trie, in order to get the order of keys.
  marisa::Agent agent;
  agent.set_query("");
  std::vector<std::unique_ptr<DictEntry>> entries;
  entries.resize(values_lexicon->Length());
  size_t maxLength = 0;
  while (dict->internal->marisa->predictive_search(agent)) {
    const std::string key(agent.key().ptr(), agent.key().length());
    size_t id = agent.key().id();
    maxLength = (std::max)(key.length(), maxLength);
    std::unique_ptr<DictEntry> entry(
        DictEntryFactory::New(key, values_lexicon->At(id)->Values()));
    entries[id] = std::move(entry);
  }
  // Read values
  dict->lexicon.reset(new Lexicon(std::move(entries)));
  dict->maxLength = maxLength;
  return dict;
}

MarisaDictPtr MarisaDict::NewFromDict(const Dict& thatDict) {
  // Extract lexicon into marisa::Keyset and a map.
  const LexiconPtr& thatLexicon = thatDict.GetLexicon();
  size_t maxLength = 0;
  marisa::Keyset keyset;
  std::unordered_map<std::string, std::unique_ptr<DictEntry>> key_value_map;
  for (size_t i = 0; i < thatLexicon->Length(); i++) {
    const DictEntry* entry = thatLexicon->At(i);
    keyset.push_back(entry->Key().c_str());
    key_value_map[entry->Key()].reset(DictEntryFactory::New(entry));
    maxLength = (std::max)(entry->KeyLength(), maxLength);
  }
  // Build Marisa Trie
  MarisaDictPtr dict(new MarisaDict());
  dict->internal->marisa->build(keyset);
  // Extract lexicon from built Marisa Trie, in order to get the order of keys.
  marisa::Agent agent;
  agent.set_query("");
  std::vector<std::unique_ptr<DictEntry>> entries;
  entries.resize(thatLexicon->Length());
  while (dict->internal->marisa->predictive_search(agent)) {
    std::string key(agent.key().ptr(), agent.key().length());
    std::unique_ptr<DictEntry> entry = std::move(key_value_map[key]);
    entries[agent.key().id()] = std::move(entry);
  }
  // Set lexicon with entries ordered by Marisa Trie key id.
  dict->lexicon.reset(new Lexicon(std::move(entries)));
  dict->maxLength = maxLength;
  return dict;
}

void MarisaDict::SerializeToFile(FILE* fp) const {
  fwrite(OCD2_HEADER, sizeof(char), strlen(OCD2_HEADER), fp);
  marisa::fwrite(fp, *internal->marisa);
  std::unique_ptr<SerializedValues> serialized_values(
      new SerializedValues(lexicon));
  serialized_values->SerializeToFile(fp);
}
