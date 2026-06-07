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

#include "PrefixMatch.hpp"
#include "Dict.hpp"
#include "Lexicon.hpp"
#include "UTF8Util.hpp"

#include <cstdint>
#include <mutex>
#include <unordered_map>

using namespace opencc;

namespace {

size_t Utf8CharLength(const char* str, size_t len) {
  if (len == 0) {
    return 0;
  }
  const size_t charLen = UTF8Util::NextCharLength(str);
  return charLen <= len ? charLen : 0;
}

uint32_t Utf8CharKey(const char* str, size_t charLen) {
  uint32_t key = static_cast<uint32_t>(charLen);
  for (size_t i = 0; i < charLen; i++) {
    key = (key << 8) | static_cast<unsigned char>(str[i]);
  }
  return key;
}

} // namespace

class PrefixMatch::Tables {
public:
  std::unique_ptr<Table> table;
};

namespace {

struct CacheEntry {
  std::vector<std::weak_ptr<const Dict>> dicts;
  std::weak_ptr<const PrefixMatch::Tables> tables;
};

bool SameOwner(const std::weak_ptr<const Dict>& cached,
               const std::weak_ptr<const Dict>& current) {
  return !cached.owner_before(current) && !current.owner_before(cached);
}

bool SameDicts(const CacheEntry& cached,
               const std::vector<std::weak_ptr<const Dict>>& current) {
  if (cached.dicts.size() != current.size()) {
    return false;
  }
  for (size_t i = 0; i < current.size(); i++) {
    if (cached.dicts[i].expired() || !SameOwner(cached.dicts[i], current[i])) {
      return false;
    }
  }
  return true;
}

bool HasExpiredDict(const CacheEntry& cached) {
  if (cached.tables.expired()) {
    return true;
  }
  for (const std::weak_ptr<const Dict>& dict : cached.dicts) {
    if (dict.expired()) {
      return true;
    }
  }
  return false;
}

void PruneExpiredPrefixMatchCache(
    std::unordered_map<std::string, std::vector<CacheEntry>>* cache) {
  for (std::unordered_map<std::string, std::vector<CacheEntry>>::iterator it =
           cache->begin();
       it != cache->end();) {
    std::vector<CacheEntry>& entries = it->second;
    for (std::vector<CacheEntry>::iterator entry = entries.begin();
         entry != entries.end();) {
      if (HasExpiredDict(*entry)) {
        entry = entries.erase(entry);
      } else {
        ++entry;
      }
    }
    if (entries.empty()) {
      it = cache->erase(it);
    } else {
      ++it;
    }
  }
}

} // namespace

class PrefixMatch::Table {
public:
  Table() {}

  void AddDict(const DictPtr& dict, size_t dictOrder) {
    const LexiconPtr lexicon = dict->GetLexicon();
    for (const std::unique_ptr<DictEntry>& item : *lexicon) {
      AddEntry(item->Key(), item->GetDefault(), dictOrder);
    }
  }

  PrefixMatch::Match MatchPrefix(const char* word, size_t len) const {
    const Node* node = &root;
    const Candidate* matchedCandidate = nullptr;
    for (const char* pstr = word; pstr < word + len;) {
      const size_t remainingLength = word + len - pstr;
      const size_t charLength = Utf8CharLength(pstr, remainingLength);
      if (charLength == 0) {
        break;
      }
      const auto child = node->children.find(Utf8CharKey(pstr, charLength));
      if (child == node->children.end()) {
        break;
      }
      pstr += charLength;
      node = child->second.get();
      if (node->candidate.hasValue &&
          (matchedCandidate == nullptr ||
           node->candidate.dictOrder < matchedCandidate->dictOrder ||
           (node->candidate.dictOrder == matchedCandidate->dictOrder &&
            node->candidate.keyLength > matchedCandidate->keyLength))) {
        matchedCandidate = &node->candidate;
      }
    }
    if (matchedCandidate != nullptr) {
      return Match{true, matchedCandidate->keyLength, &matchedCandidate->key,
                   &matchedCandidate->value};
    }
    return Match{false, 0, nullptr, nullptr};
  }

private:
  struct Candidate {
    bool hasValue = false;
    size_t dictOrder = 0;
    size_t keyLength = 0;
    std::string key;
    std::string value;
  };

  struct Node {
    Candidate candidate;
    std::unordered_map<uint32_t, std::unique_ptr<Node>> children;
  };

  void AddEntry(const std::string& key, const std::string& value,
                size_t dictOrder) {
    Node* node = &root;
    for (const char* pstr = key.c_str(); *pstr != '\0';) {
      const size_t remainingLength = key.c_str() + key.length() - pstr;
      const size_t charLength = Utf8CharLength(pstr, remainingLength);
      if (charLength == 0) {
        break;
      }
      std::unique_ptr<Node>& child =
          node->children[Utf8CharKey(pstr, charLength)];
      if (child == nullptr) {
        child.reset(new Node);
      }
      node = child.get();
      pstr += charLength;
    }
    if (!node->candidate.hasValue || dictOrder < node->candidate.dictOrder) {
      node->candidate.hasValue = true;
      node->candidate.dictOrder = dictOrder;
      node->candidate.keyLength = key.length();
      node->candidate.key = key;
      node->candidate.value = value;
    }
  }

  Node root;
};

PrefixMatch::PrefixMatch(const DictPtr& dict) {
  static std::mutex cacheMutex;
  static std::unordered_map<std::string, std::vector<CacheEntry>> cache;

  std::string cacheKey;
  AppendCacheKey(dict, &cacheKey);
  std::vector<std::weak_ptr<const Dict>> leafDicts;
  CollectLeafDicts(dict, &leafDicts);

  {
    std::lock_guard<std::mutex> lock(cacheMutex);
    PruneExpiredPrefixMatchCache(&cache);
    const auto cached = cache.find(cacheKey);
    if (cached != cache.end()) {
      for (const CacheEntry& entry : cached->second) {
        if (SameDicts(entry, leafDicts)) {
          tables = entry.tables.lock();
          if (tables != nullptr) {
            return;
          }
        }
      }
    }
  }

  std::shared_ptr<Tables> built(new Tables);
  built->table.reset(new Table);
  size_t dictOrder = 0;
  AddDict(dict, built.get(), &dictOrder);

  std::lock_guard<std::mutex> lock(cacheMutex);
  PruneExpiredPrefixMatchCache(&cache);
  std::vector<CacheEntry>& entries = cache[cacheKey];
  for (std::vector<CacheEntry>::iterator it = entries.begin();
       it != entries.end();) {
    if (HasExpiredDict(*it)) {
      it = entries.erase(it);
    } else if (SameDicts(*it, leafDicts)) {
      tables = it->tables.lock();
      if (tables != nullptr) {
        return;
      }
      it = entries.erase(it);
    } else {
      ++it;
    }
  }
  tables = built;
  CacheEntry entry;
  entry.dicts = std::move(leafDicts);
  entry.tables = tables;
  entries.push_back(std::move(entry));
}

PrefixMatch::~PrefixMatch() {}

PrefixMatch::Match PrefixMatch::MatchPrefix(const char* word,
                                            size_t len) const {
  return tables->table->MatchPrefix(word, len);
}

void PrefixMatch::AddDict(const DictPtr& dict, Tables* output,
                          size_t* dictOrder) {
  const std::list<DictPtr>* dictGroupItems = dict->GetDictGroupItems();
  if (dictGroupItems != nullptr) {
    for (const DictPtr& child : *dictGroupItems) {
      AddDict(child, output, dictOrder);
    }
    return;
  }
  output->table->AddDict(dict, *dictOrder);
  ++(*dictOrder);
}

void PrefixMatch::AppendCacheKey(const DictPtr& dict, std::string* output) {
  const std::list<DictPtr>* dictGroupItems = dict->GetDictGroupItems();
  if (dictGroupItems != nullptr) {
    output->push_back('[');
    for (const DictPtr& child : *dictGroupItems) {
      AppendCacheKey(child, output);
    }
    output->push_back(']');
    return;
  }

  const uintptr_t dictKey = reinterpret_cast<uintptr_t>(dict.get());
  output->append(reinterpret_cast<const char*>(&dictKey), sizeof(dictKey));
  output->push_back(';');
}

void PrefixMatch::CollectLeafDicts(
    const DictPtr& dict, std::vector<std::weak_ptr<const Dict>>* output) {
  const std::list<DictPtr>* dictGroupItems = dict->GetDictGroupItems();
  if (dictGroupItems != nullptr) {
    for (const DictPtr& child : *dictGroupItems) {
      CollectLeafDicts(child, output);
    }
    return;
  }
  output->push_back(dict);
}
