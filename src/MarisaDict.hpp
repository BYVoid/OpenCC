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

#pragma once

#include <atomic>
#include <mutex>

#include "Common.hpp"
#include "SerializableDict.hpp"

namespace marisa {
class Trie;
}

namespace opencc {
/**
 * Darts dictionary
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT MarisaDict : public Dict, public SerializableDict {
public:
  virtual ~MarisaDict() override;

  virtual size_t KeyMaxLength() const override;

  virtual Optional<const DictEntry*> Match(const char* word,
                                           size_t len) const override;

  virtual Optional<const DictEntry*> MatchPrefix(const char* word,
                                                 size_t len) const override;

  virtual std::vector<const DictEntry*> MatchAllPrefixes(
      const char* word, size_t len) const override;

  virtual LexiconPtr GetLexicon() const override;

  virtual bool SupportsFastPrefixMatch() const override { return true; }

  virtual PrefixMatchView MatchPrefixValue(const char* word,
                                           size_t len) const override;

  virtual void SerializeToFile(FILE* fp) const override;

  /**
   * Constructs a MarisaDict from another dictionary.
   */
  static MarisaDictPtr NewFromDict(const Dict& thatDict);

  static MarisaDictPtr NewFromFile(FILE* fp);

  static MarisaDictPtr NewFromBuffer(const char* data, size_t size);

  // Exposed for testing only.
  bool IsLexiconReconstructed() const {
    return lexiconReconstructed.load(std::memory_order_acquire);
  }

private:
  MarisaDict();

  void LoadFromMappedBuffer();
  void ReconstructLexicon() const;

  mutable size_t maxLength;
  mutable LexiconPtr lexicon;
  mutable std::mutex lexiconMutex;
  mutable std::atomic<bool> lexiconReconstructed;
  LexiconPtr valuesLexicon;

  class MarisaInternal;
  std::unique_ptr<MarisaInternal> internal;
};
} // namespace opencc
