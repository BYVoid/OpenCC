/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 Carbo Kuo <byvoid@byvoid.com>
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

#include <cstdint>
#include <cstring>

#include "Common.hpp"
#include "SerializableDict.hpp"

namespace opencc {
// Returns true if the 8 bytes at `bytes` look like an 8-byte integer field
// from the legacy 64-bit OCD layout rather than two fixed-width uint32_t
// fields. Legacy files were written with native size_t fields in native byte
// order, so the probe is loaded as a native uint64_t: a legacy field has
// zero high 32 bits, while for a fixed-width file the high half is one of
// the two uint32 fields (or the start of the darts array), non-zero for any
// realistic dictionary. Works on both little- and big-endian hosts; legacy
// files themselves remain readable only on hosts of the byte order that
// wrote them.
inline bool LooksLikeLegacy64Field(const void* bytes) {
  uint64_t word;
  std::memcpy(&word, bytes, sizeof(word));
  return (word >> 32) == 0;
}

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
  static BinaryDictPtr NewFromBuffer(const char* data, size_t size);

  const LexiconPtr& GetLexicon() const { return lexicon; }

  size_t KeyMaxLength() const;

private:
  LexiconPtr lexicon;
  std::string keyBuffer;
  std::string valueBuffer;

  void ConstructBuffer(std::string& keyBuffer, std::vector<size_t>& keyOffset,
                       size_t& keyTotalLength, std::string& valueBuffer,
                       std::vector<size_t>& valueOffset,
                       size_t& valueTotalLength) const;
};
} // namespace opencc
