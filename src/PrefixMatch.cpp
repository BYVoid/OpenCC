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
#include "DictGroup.hpp"
#include "Lexicon.hpp"
#include "UTF8Util.hpp"

#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

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
  class Matcher;
  std::unique_ptr<Matcher> matcher;
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

void Unreachable() {
#if defined(_MSC_VER)
  __assume(false);
#elif defined(__GNUC__) || defined(__clang__)
  __builtin_unreachable();
#endif
}

} // namespace

class PrefixMatch::Tables::Matcher {
public:
  struct Candidate {
    bool hasValue = false;
    size_t keyLength = 0;
    const std::string* key = nullptr;
    const std::string* value = nullptr;
  };

  virtual ~Matcher() {}

  virtual Candidate MatchPrefixCandidate(const char* word,
                                         size_t len) const = 0;
};

class LeafMatcher : public PrefixMatch::Tables::Matcher {
private:
  struct StoredCandidate {
    bool hasValue = false;
    size_t keyLength = 0;
    std::string key;
    std::string value;
  };

  struct Node {
    StoredCandidate candidate;
    std::unordered_map<uint32_t, std::unique_ptr<Node>> children;
  };

public:
  LeafMatcher() {}

  void AddDict(const DictPtr& dict) {
    const LexiconPtr lexicon = dict->GetLexicon();
    for (const std::unique_ptr<DictEntry>& item : *lexicon) {
      AddEntry(item->Key(), item->GetDefault());
    }
  }

  Candidate MatchPrefixCandidate(const char* word, size_t len) const override {
    const Node* node = &root;
    const StoredCandidate* matchedCandidate = nullptr;
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
           node->candidate.keyLength > matchedCandidate->keyLength)) {
        matchedCandidate = &node->candidate;
      }
    }
    if (matchedCandidate == nullptr) {
      return Candidate{};
    }
    return Candidate{true, matchedCandidate->keyLength,
                     &matchedCandidate->key, &matchedCandidate->value};
  }

private:
  void AddEntry(const std::string& key, const std::string& value) {
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
    if (!node->candidate.hasValue) {
      node->candidate.hasValue = true;
      node->candidate.keyLength = key.length();
      node->candidate.key = key;
      node->candidate.value = value;
    }
  }

  Node root;
};

class GroupMatcher : public PrefixMatch::Tables::Matcher {
public:
  explicit GroupMatcher(DictGroupMatchPolicy _matchPolicy)
      : matchPolicy(_matchPolicy) {}

  void AddChild(std::unique_ptr<Matcher> child) {
    children.push_back(std::move(child));
  }

  Candidate MatchPrefixCandidate(const char* word, size_t len) const override {
    switch (matchPolicy) {
    case DictGroupMatchPolicy::ShortCircuit:
      return MatchPrefixShortCircuit(word, len);
    case DictGroupMatchPolicy::Union:
      return MatchPrefixUnion(word, len);
    }
    Unreachable();
    return Candidate{};
  }

private:
  Candidate MatchPrefixShortCircuit(const char* word, size_t len) const {
    for (const std::unique_ptr<Matcher>& child : children) {
      const Candidate candidate = child->MatchPrefixCandidate(word, len);
      if (candidate.hasValue) {
        return candidate;
      }
    }
    return Candidate{};
  }

  Candidate MatchPrefixUnion(const char* word, size_t len) const {
    Candidate best;
    for (const std::unique_ptr<Matcher>& child : children) {
      const Candidate candidate = child->MatchPrefixCandidate(word, len);
      if (candidate.hasValue &&
          (!best.hasValue || candidate.keyLength > best.keyLength)) {
        best = candidate;
      }
    }
    return best;
  }

  std::vector<std::unique_ptr<Matcher>> children;
  const DictGroupMatchPolicy matchPolicy;
};

std::unique_ptr<PrefixMatch::Tables::Matcher> BuildMatcher(
    const DictPtr& dict) {
  const std::list<DictPtr>* dictGroupItems = dict->GetDictGroupItems();
  if (dictGroupItems != nullptr) {
    std::unique_ptr<GroupMatcher> group(
        new GroupMatcher(dict->GetMatchPolicy()));
    for (const DictPtr& child : *dictGroupItems) {
      group->AddChild(BuildMatcher(child));
    }
    return std::move(group);
  }

  std::unique_ptr<LeafMatcher> leaf(new LeafMatcher);
  leaf->AddDict(dict);
  return std::move(leaf);
}

PrefixMatch::PrefixMatch(const DictPtr& dict) {
  // Try to unwrap single dict group
  DictPtr actualDict = dict;
  while (actualDict) {
    const std::list<DictPtr>* items = actualDict->GetDictGroupItems();
    if (items != nullptr && items->size() == 1) {
      actualDict = items->front();
    } else {
      break;
    }
  }

  if (actualDict && actualDict->SupportsFastPrefixMatch()) {
    singleDict = actualDict;
    return;
  }

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
  built->matcher = BuildMatcher(dict);

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
  if (singleDict != nullptr) {
    struct MatchCache {
      std::string key;
      std::string value;
    };
    // key/value pointers are valid until the next MatchPrefix() call on this
    // thread.
    static thread_local MatchCache matchCache;
    const PrefixMatchView pv = singleDict->MatchPrefixValue(word, len);
    if (pv.matched) {
      matchCache.key = std::string(pv.key);
      matchCache.value = std::string(pv.value);
      return Match{true, pv.keyLength, &matchCache.key, &matchCache.value};
    }
    return Match{false, 0, nullptr, nullptr};
  }

  const Tables::Matcher::Candidate candidate =
      tables->matcher->MatchPrefixCandidate(word, len);
  if (candidate.hasValue) {
    return Match{true, candidate.keyLength, candidate.key, candidate.value};
  }
  return Match{false, 0, nullptr, nullptr};
}

PrefixMatchView PrefixMatch::MatchPrefixView(const char* word,
                                              size_t len) const {
  if (singleDict != nullptr) {
    return singleDict->MatchPrefixValue(word, len);
  }
  const Tables::Matcher::Candidate candidate =
      tables->matcher->MatchPrefixCandidate(word, len);
  if (candidate.hasValue) {
    return {true, candidate.keyLength,
            std::string_view(*candidate.key),
            std::string_view(*candidate.value)};
  }
  return {false, 0, std::string_view(), std::string_view()};
}

void PrefixMatch::AppendCacheKey(const DictPtr& dict, std::string* output) {
  const std::list<DictPtr>* dictGroupItems = dict->GetDictGroupItems();
  if (dictGroupItems != nullptr) {
    output->push_back('[');
    const DictGroupMatchPolicy matchPolicy = dict->GetMatchPolicy();
    switch (matchPolicy) {
    case DictGroupMatchPolicy::ShortCircuit:
      output->append("short_circuit:");
      break;
    case DictGroupMatchPolicy::Union:
      output->append("union:");
      break;
    }
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
