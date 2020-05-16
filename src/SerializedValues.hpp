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
 * Binary format for dictionary values serialization.
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT SerializedValues : public SerializableDict {
public:
  SerializedValues(const LexiconPtr& _lexicon) : lexicon(_lexicon) {}

  virtual ~SerializedValues() {}

  virtual void SerializeToFile(FILE* fp) const;

  static std::shared_ptr<SerializedValues> NewFromFile(FILE* fp);

  const LexiconPtr& GetLexicon() const { return lexicon; }

  size_t KeyMaxLength() const;

private:
  LexiconPtr lexicon;

  void ConstructBuffer(std::string* valueBuffer,
                       std::vector<uint16_t>* valueBytes,
                       uint32_t* valueTotalLength) const;
};
} // namespace opencc
