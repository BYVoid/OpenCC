/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 BYVoid <byvoid@byvoid.com>
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
* Binary dictionary for faster deserialization
* @ingroup opencc_cpp_api
*/
class OPENCC_EXPORT BinaryDict : public SerializableDict {
public:
  BinaryDict(const LexiconPtr& _lexicon) : lexicon(_lexicon) {}

  virtual ~BinaryDict() {}

  virtual void SerializeToFile(FILE* fp) const;

  static BinaryDictPtr NewFromFile(FILE* fp);

  const LexiconPtr& GetLexicon() const { return lexicon; }

  size_t KeyMaxLength() const;

private:
  LexiconPtr lexicon;
  string keyBuffer;
  string valueBuffer;

  void ConstructBuffer(string& keyBuffer, vector<size_t>& keyOffset,
                       size_t& keyTotalLength, string& valueBuffer,
                       vector<size_t>& valueOffset,
                       size_t& valueTotalLength) const;
};
}
