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

#include "Common.hpp"
#include "SerializableDict.hpp"

namespace opencc {
/**
 * Darts dictionary
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT MarisaDict : public Dict, public SerializableDict {
public:
  virtual ~MarisaDict();

  virtual size_t KeyMaxLength() const;

  virtual Optional<const DictEntry*> Match(const char* word, size_t len) const;

  virtual Optional<const DictEntry*> MatchPrefix(const char* word,
                                                 size_t len) const;

  virtual std::vector<const DictEntry*> MatchAllPrefixes(const char* word,
                                                         size_t len) const;

  virtual LexiconPtr GetLexicon() const;

  virtual void SerializeToFile(FILE* fp) const;

  /**
   * Constructs a MarisaDict from another dictionary.
   */
  static MarisaDictPtr NewFromDict(const Dict& thatDict);

  static MarisaDictPtr NewFromFile(FILE* fp);

private:
  MarisaDict();

  size_t maxLength;
  LexiconPtr lexicon;

  class MarisaInternal;
  std::unique_ptr<MarisaInternal> internal;
};
} // namespace opencc
