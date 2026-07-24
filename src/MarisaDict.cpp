/*
 * Open Chinese Convert
 *
 * Copyright 2020-2026 Carbo Kuo and contributors
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
#include <stdexcept>
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
  std::string mappedBuffer;

  MarisaInternal() : marisa(new marisa::Trie()) {}
};

MarisaDict::MarisaDict()
    : maxLength(128), lexiconReconstructed(false), internal(new MarisaInternal()) {}

MarisaDict::~MarisaDict() {}

size_t MarisaDict::KeyMaxLength() const {
  ReconstructLexicon();
  return maxLength;
}

Optional<const DictEntry*> MarisaDict::Match(const char* word,
                                             size_t len) const {
  ReconstructLexicon();
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
  ReconstructLexicon();
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
  ReconstructLexicon();
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

LexiconPtr MarisaDict::GetLexicon() const {
  ReconstructLexicon();
  return lexicon;
}

void MarisaDict::ReconstructLexicon() const {
  if (lexiconReconstructed.load(std::memory_order_acquire)) {
    return;
  }
  std::lock_guard<std::mutex> lock(lexiconMutex);
  if (lexiconReconstructed.load(std::memory_order_relaxed)) {
    return;
  }
  if (!valuesLexicon) {
    lexiconReconstructed.store(true, std::memory_order_release);
    return;
  }
  marisa::Agent agent;
  agent.set_query("");
  std::vector<std::unique_ptr<DictEntry>> entries;
  entries.resize(valuesLexicon->Length());
  size_t maxLen = 0;
  try {
    while (internal->marisa->predictive_search(agent)) {
      const std::string key(agent.key().ptr(), agent.key().length());
      size_t id = agent.key().id();
      if (id >= entries.size()) {
        throw InvalidFormat(
            "Invalid OpenCC Marisa dictionary (key id out of bounds)");
      }
      maxLen = (std::max)(key.length(), maxLen);
      std::unique_ptr<DictEntry> entry(
          DictEntryFactory::New(key, valuesLexicon->At(id)->Values()));
      entries[id] = std::move(entry);
    }
  } catch (const std::exception& e) {
    throw InvalidFormat(std::string("Invalid OpenCC Marisa dictionary: ") +
                        e.what());
  }
  lexicon.reset(new Lexicon(std::move(entries)));
  maxLength = maxLen;
  lexiconReconstructed.store(true, std::memory_order_release);
}

MarisaDictPtr MarisaDict::NewFromFile(FILE* fp) {
  // Verify file header
  size_t headerLen = strlen(OCD2_HEADER);
  void* buffer = malloc(sizeof(char) * headerLen);
  size_t bytesRead = fread(buffer, sizeof(char), headerLen, fp);
  if (bytesRead != headerLen || memcmp(buffer, OCD2_HEADER, headerLen) != 0) {
    throw InvalidFormat("Invalid OpenCC dictionary header");
  }
  free(buffer);
  long trieOffset = ftell(fp);
  fseek(fp, 0L, SEEK_END);
  long fileEnd = ftell(fp);
  fseek(fp, trieOffset, SEEK_SET);
  size_t remainingSize =
      (fileEnd > trieOffset) ? static_cast<size_t>(fileEnd - trieOffset) : 0;

  MarisaDictPtr dict(new MarisaDict());
  if (remainingSize > 0) {
    dict->internal->mappedBuffer.resize(remainingSize);
    bytesRead = fread(&dict->internal->mappedBuffer[0],
                      sizeof(char), remainingSize, fp);
    if (bytesRead != remainingSize) {
      throw InvalidFormat("Invalid OpenCC Marisa dictionary.");
    }
  }

  dict->LoadFromMappedBuffer();
  return dict;
}

MarisaDictPtr MarisaDict::NewFromBuffer(const char* data, size_t size) {
  // Verify file header
  size_t headerLen = strlen(OCD2_HEADER);
  if (size < headerLen || memcmp(data, OCD2_HEADER, headerLen) != 0) {
    throw InvalidFormat("Invalid OpenCC dictionary header");
  }

  size_t remainingSize = size - headerLen;
  MarisaDictPtr dict(new MarisaDict());
  dict->internal->mappedBuffer.assign(data + headerLen, remainingSize);

  dict->LoadFromMappedBuffer();
  return dict;
}

void MarisaDict::LoadFromMappedBuffer() {
  try {
    internal->marisa->map(internal->mappedBuffer.data(),
                          internal->mappedBuffer.size());
  } catch (const std::exception& e) {
    throw InvalidFormat(std::string("Invalid OpenCC Marisa dictionary: ") +
                        e.what());
  }

  const size_t trieSize = internal->marisa->io_size();
  if (trieSize > internal->mappedBuffer.size()) {
    throw InvalidFormat(
        "Invalid OpenCC Marisa dictionary (trie exceeds file size)");
  }

  size_t valuesBytesRead = 0;
  std::shared_ptr<SerializedValues> serialized_values =
      SerializedValues::NewFromBuffer(
          internal->mappedBuffer.data() + trieSize,
          internal->mappedBuffer.size() - trieSize, &valuesBytesRead);
  valuesLexicon = serialized_values->GetLexicon();
  // Validate key count consistency
  size_t numKeys = internal->marisa->num_keys();
  if (numKeys != valuesLexicon->Length()) {
    throw InvalidFormat(
        "Invalid OpenCC Marisa dictionary (key count mismatch)");
  }
  maxLength = 128;
  lexiconReconstructed.store(false, std::memory_order_release);
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
  dict->lexiconReconstructed.store(true, std::memory_order_release);
  return dict;
}

void MarisaDict::SerializeToFile(FILE* fp) const {
  fwrite(OCD2_HEADER, sizeof(char), strlen(OCD2_HEADER), fp);
  marisa::fwrite(fp, *internal->marisa);
  std::unique_ptr<SerializedValues> serialized_values(
      new SerializedValues(GetLexicon()));
  serialized_values->SerializeToFile(fp);
}

bool MarisaDict::EnumerateKeys(
    const std::function<void(const char*, size_t)>& cb) const {
  // Prefer the already-reconstructed lexicon (e.g. when a table-path
  // PrefixMatch materialized it); fall back to walking the trie so lazy
  // loading is never forced. Same convention as ReconstructLexicon():
  // lexicon is written once before the release store of the flag, so the
  // acquire load makes the plain read safe.
  if (lexiconReconstructed.load(std::memory_order_acquire) &&
      lexicon != nullptr) {
    for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
      const std::string& key = entry->Key();
      cb(key.data(), key.length());
    }
    return true;
  }
  const marisa::Trie* trie = internal->marisa.get();
  if (trie == nullptr) {
    return false;
  }
  try {
    marisa::Agent agent;
    agent.set_query("");
    while (trie->predictive_search(agent)) {
      cb(agent.key().ptr(), agent.key().length());
    }
  } catch (const marisa::Exception&) {
    // The trie has not been built or mapped yet.
    return false;
  }
  return true;
}

PrefixMatchView MarisaDict::MatchPrefixValue(const char* word,
                                             size_t len) const {
  const marisa::Trie* trie = internal->marisa.get();
  if (trie == nullptr) {
    return PrefixMatchView{};
  }
  static thread_local marisa::Agent agent;
  agent.set_query(word, len);
  bool matched = false;
  size_t matchedId = 0;
  size_t matchedLength = 0;
  while (trie->common_prefix_search(agent)) {
    const size_t currentLength = agent.key().length();
    if (!matched || currentLength > matchedLength) {
      matched = true;
      matchedId = agent.key().id();
      matchedLength = currentLength;
    }
  }
  if (!matched) {
    return PrefixMatchView{};
  }
  // value view points directly into the DictEntry's owned string storage,
  // valid for the lifetime of this dictionary.
  if (valuesLexicon != nullptr) {
    if (matchedId < valuesLexicon->Length()) {
      return PrefixMatchView{true, matchedLength,
                             std::string_view(word, matchedLength),
                             valuesLexicon->At(matchedId)->GetDefaultView()};
    }
    return PrefixMatchView{};
  }
  LexiconPtr lex = GetLexicon();
  if (lex != nullptr && matchedId < lex->Length()) {
    return PrefixMatchView{true, matchedLength,
                           std::string_view(word, matchedLength),
                           lex->At(matchedId)->GetDefaultView()};
  }
  return PrefixMatchView{};
}
