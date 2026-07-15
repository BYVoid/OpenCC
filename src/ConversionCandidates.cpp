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

#include "ConversionCandidates.hpp"

#include <unordered_set>

#include "Common.hpp"
#include "ConfigBasedConverter.hpp"
#include "Conversion.hpp"
#include "ConversionChain.hpp"
#include "Converter.hpp"
#include "Dict.hpp"
#include "DictEntry.hpp"
#include "Optional.hpp"

namespace opencc {

std::vector<std::string> GetAllConversions(const Converter& converter,
                                           std::string_view word) {
  const ConversionChainPtr chain = converter.GetConversionChain();
  if (chain == nullptr) {
    return {};
  }

  // A config-based converter runs a normalization pre-pass (e.g. CJK
  // Compatibility Ideographs) before its main chain, but GetConversionChain()
  // exposes only the main chain. Mirror Convert() by normalizing first, so a
  // word whose dictionary key appears only after normalization is still found.
  std::string normalized;
  if (const auto* configBased =
          dynamic_cast<const ConfigBasedConverter*>(&converter)) {
    normalized = configBased->GetNormalizationConverter()->Convert(word);
    word = normalized;
  }

  std::vector<std::string> currentWords{std::string(word)};
  bool matched = false;
  for (const ConversionPtr& conversion : chain->GetConversions()) {
    const DictPtr dict = conversion->GetDict();
    if (dict == nullptr) {
      return {};
    }
    std::unordered_set<std::string> seen;
    std::vector<std::string> nextWords;
    for (const std::string& currentWord : currentWords) {
      const Optional<const DictEntry*> item = dict->Match(currentWord);
      if (item.IsNull()) {
        // No exact match: convert greedily by longest prefix so a partially
        // convertible word can still flow through the rest of the chain. Even
        // when the result is unchanged we keep it, because a later dictionary
        // may still convert it. Conversion::Convert also handles ideographic
        // description sequences and truncated UTF-8 safely.
        std::string buffer = conversion->Convert(currentWord);
        if (seen.insert(buffer).second) {
          nextWords.push_back(std::move(buffer));
        }
        continue;
      }
      matched = true;
      if (item.Get()->NumValues() == 0) {
        // A key-only entry converts the word to itself, matching Convert(),
        // which falls back to the key via DictEntry::GetDefault().
        if (seen.insert(currentWord).second) {
          nextWords.push_back(currentWord);
        }
        continue;
      }
      for (const std::string& value : item.Get()->Values()) {
        if (seen.insert(value).second) {
          nextWords.push_back(value);
        }
      }
    }
    currentWords.swap(nextWords);
  }

  if (!matched) {
    // No dictionary in the chain contains the word.
    return {};
  }
  return currentWords;
}

} // namespace opencc
