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

class PrefixMatch::Table {
public:
  explicit Table(const DictPtr& dict) {
    const LexiconPtr lexicon = dict->GetLexicon();
    for (const std::unique_ptr<DictEntry>& item : *lexicon) {
      AddEntry(item->Key(), item->GetDefault());
    }
  }

  PrefixMatch::Match MatchPrefix(const char* word, size_t len) const {
    const Node* node = &root;
    const Node* matchedNode = nullptr;
    size_t matchedLength = 0;
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
      if (node->keyLength > 0) {
        matchedLength = node->keyLength;
        matchedNode = node;
      }
    }
    if (matchedNode != nullptr) {
      return Match{true, matchedLength, &matchedNode->key,
                   &matchedNode->value};
    }
    return Match{false, 0, nullptr, nullptr};
  }

private:
  struct Node {
    size_t keyLength = 0;
    std::string key;
    std::string value;
    std::unordered_map<uint32_t, std::unique_ptr<Node>> children;
  };

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
    node->keyLength = key.length();
    node->key = key;
    node->value = value;
  }

  Node root;
};

PrefixMatch::PrefixMatch(const DictPtr& dict) { AddDict(dict); }

PrefixMatch::~PrefixMatch() {}

PrefixMatch::Match PrefixMatch::MatchPrefix(const char* word,
                                            size_t len) const {
  for (const std::unique_ptr<Table>& table : tables) {
    const Match match = table->MatchPrefix(word, len);
    if (match.matched) {
      return match;
    }
  }
  return Match{false, 0, nullptr, nullptr};
}

void PrefixMatch::AddDict(const DictPtr& dict) {
  const std::list<DictPtr>* dictGroupItems = dict->GetDictGroupItems();
  if (dictGroupItems != nullptr) {
    for (const DictPtr& child : *dictGroupItems) {
      AddDict(child);
    }
    return;
  }
  tables.emplace_back(new Table(dict));
}
