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

#include "Conversion.hpp"
#include "Dict.hpp"
#include "Lexicon.hpp"

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

class opencc::LinearMatchDict {
public:
  struct Match {
    bool matched;
    size_t keyLength;
    const std::string* value;
  };

  explicit LinearMatchDict(const DictPtr& dict) { AddDict(dict); }

  Match MatchPrefix(const char* word, size_t len) const {
    for (const Table& table : tables) {
      const Match match = table.MatchPrefix(word, len);
      if (match.matched) {
        return match;
      }
    }
    return Match{false, 0, nullptr};
  }

private:
  struct Entry {
    std::string key;
    std::string value;
  };

  class Table {
  public:
    explicit Table(const DictPtr& dict) {
      const LexiconPtr lexicon = dict->GetLexicon();
      for (const std::unique_ptr<DictEntry>& item : *lexicon) {
        AddEntry(item->Key(), item->GetDefault());
      }
    }

    Match MatchPrefix(const char* word, size_t len) const {
      const Node* node = &root;
      const std::string* matchedValue = nullptr;
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
          matchedValue = &node->value;
        }
      }
      if (matchedValue != nullptr) {
        return Match{true, matchedLength, matchedValue};
      }
      return Match{false, 0, nullptr};
    }

  private:
    struct Node {
      size_t keyLength = 0;
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
      node->value = value;
    }

    Node root;
  };

  void AddDict(const DictPtr& dict) {
    const std::list<DictPtr>* dictGroupItems = dict->GetDictGroupItems();
    if (dictGroupItems != nullptr) {
      for (const DictPtr& child : *dictGroupItems) {
        AddDict(child);
      }
      return;
    }
    tables.emplace_back(dict);
  }

  std::vector<Table> tables;
};

Conversion::Conversion(DictPtr _dict)
    : dict(_dict), linearDict(new LinearMatchDict(_dict)) {}

std::string Conversion::Convert(const char* phrase) const {
  std::string buffer;
  AppendConverted(phrase, &buffer);
  return buffer;
}

void Conversion::AppendConverted(const char* phrase, std::string* output) const {
  // Calculate string end to prevent reading beyond null terminator
  const char* phraseEnd = phrase;
  while (*phraseEnd != '\0') {
    phraseEnd++;
  }
  const size_t phraseLength = phraseEnd - phrase;
  output->reserve(output->size() + phraseLength + phraseLength / 5);

  for (const char* pstr = phrase; *pstr != '\0';) {
    size_t remainingLength = phraseEnd - pstr;
    const LinearMatchDict::Match matched =
        linearDict->MatchPrefix(pstr, remainingLength);
    size_t matchedLength;
    if (!matched.matched) {
      matchedLength = UTF8Util::NextCharLength(pstr);
      // Ensure we don't read beyond the null terminator
      if (matchedLength > remainingLength) {
        matchedLength = remainingLength;
      }
      output->append(pstr, matchedLength);
    } else {
      matchedLength = matched.keyLength;
      // Defensive: ensure dictionary key length does not exceed remaining input
      // (MatchPrefix should already guarantee this, but defense in depth)
      if (matchedLength > remainingLength) {
        matchedLength = remainingLength;
      }
      output->append(*matched.value);
    }
    pstr += matchedLength;
  }
}

std::string Conversion::Convert(const std::string& phrase) const {
  return Convert(phrase.c_str());
}

SegmentsPtr Conversion::Convert(const SegmentsPtr& input) const {
  SegmentsPtr output(new Segments);
  for (const char* segment : *input) {
    output->AddSegment(Convert(segment));
  }
  return output;
}
