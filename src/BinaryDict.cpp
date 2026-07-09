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

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>

#include "BinaryDict.hpp"
#include "Lexicon.hpp"

using namespace opencc;

size_t BinaryDict::KeyMaxLength() const {
  size_t maxLength = 0;
  for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
    maxLength = (std::max)(maxLength, entry->KeyLength());
  }
  return maxLength;
}

namespace {

// All integer fields are serialized as fixed-width uint32_t so that the file
// is identical regardless of the word size of the platform that built it.
// Files written by old 64-bit builds used native 8-byte size_t fields; the
// deserializers below detect and accept both layouts.
void WriteField(FILE* fp, size_t value) {
  if (value > UINT32_MAX) {
    throw InvalidFormat("OpenCC binary dictionary field exceeds uint32 range");
  }
  uint32_t field = static_cast<uint32_t>(value);
  fwrite(&field, sizeof(field), 1, fp);
}

uint64_t ReadField(const char* data, size_t size, size_t* offset,
                   size_t fieldWidth) {
  if (fieldWidth > size || *offset > size - fieldWidth) {
    throw InvalidFormat("Invalid OpenCC binary dictionary (truncated)");
  }
  uint64_t value;
  if (fieldWidth == sizeof(uint32_t)) {
    uint32_t field;
    memcpy(&field, data + *offset, sizeof(field));
    value = field;
  } else {
    memcpy(&value, data + *offset, sizeof(value));
  }
  *offset += fieldWidth;
  return value;
}

// Detects the integer field width of a serialized BinaryDict.
//   - Fixed-width layout (current builds and old 32-bit builds): the first 8
//     bytes are numItems followed by keyTotalLength; keyTotalLength is
//     non-zero whenever the dictionary has entries, and an empty dictionary
//     is exactly 12 bytes long.
//   - Legacy 64-bit layout: the first field is an 8-byte numItems whose high
//     4 bytes are zero for any realistic dictionary.
size_t DetectFieldWidth(const char* data, size_t size) {
  if (size < 8) {
    return sizeof(uint32_t);
  }
  const bool highBytesZero =
      data[4] == 0 && data[5] == 0 && data[6] == 0 && data[7] == 0;
  if (highBytesZero && size != 3 * sizeof(uint32_t)) {
    return sizeof(uint64_t);
  }
  return sizeof(uint32_t);
}

} // namespace

void BinaryDict::SerializeToFile(FILE* fp) const {
  std::string keyBuf, valueBuf;
  std::vector<size_t> keyOffsets, valueOffsets;
  size_t keyTotalLength = 0, valueTotalLength = 0;
  ConstructBuffer(keyBuf, keyOffsets, keyTotalLength, valueBuf, valueOffsets,
                  valueTotalLength);
  // Number of items
  size_t numItems = lexicon->Length();
  WriteField(fp, numItems);

  // Data
  WriteField(fp, keyTotalLength);
  fwrite(keyBuf.c_str(), sizeof(char), keyTotalLength, fp);
  WriteField(fp, valueTotalLength);
  fwrite(valueBuf.c_str(), sizeof(char), valueTotalLength, fp);

  size_t keyCursor = 0, valueCursor = 0;
  for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
    // Number of values
    size_t numValues = entry->NumValues();
    WriteField(fp, numValues);
    // Key offset
    size_t keyOffset = keyOffsets[keyCursor++];
    WriteField(fp, keyOffset);
    // Values offset
    for (size_t i = 0; i < numValues; i++) {
      size_t valueOffset = valueOffsets[valueCursor++];
      WriteField(fp, valueOffset);
    }
  }
  assert(keyCursor == numItems);
}

BinaryDictPtr BinaryDict::NewFromFile(FILE* fp) {
  long savedOffset = ftell(fp);
  fseek(fp, 0L, SEEK_END);
  long remainingSizeLong = ftell(fp) - savedOffset;
  fseek(fp, savedOffset, SEEK_SET);
  assert(remainingSizeLong >= 0);
  size_t remainingSize = static_cast<size_t>(remainingSizeLong);

  std::string buffer;
  buffer.resize(remainingSize);
  size_t bytesRead = fread(const_cast<char*>(buffer.c_str()), sizeof(char),
                           remainingSize, fp);
  if (bytesRead != remainingSize) {
    throw InvalidFormat("Invalid OpenCC binary dictionary (truncated)");
  }
  return NewFromBuffer(buffer.data(), buffer.size());
}

BinaryDictPtr BinaryDict::NewFromBuffer(const char* data, size_t size) {
  BinaryDictPtr dict(new BinaryDict(LexiconPtr(new Lexicon)));
  size_t offset = 0;
  const size_t fieldWidth = DetectFieldWidth(data, size);

  auto readField = [&]() -> size_t {
    uint64_t value = ReadField(data, size, &offset, fieldWidth);
    if (value > size) {
      // Every field is a count, length or offset bounded by the data size;
      // this also guarantees the value fits in size_t on 32-bit platforms.
      throw InvalidFormat("Invalid OpenCC binary dictionary (field too large)");
    }
    return static_cast<size_t>(value);
  };

  size_t numItems = readField();

  size_t keyTotalLength = readField();
  if (keyTotalLength > size - offset) {
    throw InvalidFormat(
        "Invalid OpenCC binary dictionary (keyTotalLength exceeds data size)");
  }
  dict->keyBuffer.assign(data + offset, keyTotalLength);
  offset += keyTotalLength;

  size_t valueTotalLength = readField();
  if (valueTotalLength > size - offset) {
    throw InvalidFormat(
        "Invalid OpenCC binary dictionary (valueTotalLength exceeds data size)");
  }
  dict->valueBuffer.assign(data + offset, valueTotalLength);
  offset += valueTotalLength;

  for (size_t i = 0; i < numItems; i++) {
    size_t numValues = readField();
    size_t keyOffset = readField();
    if (keyOffset >= keyTotalLength) {
      throw InvalidFormat("Invalid OpenCC binary dictionary (keyOffset)");
    }
    const char* keyStart = dict->keyBuffer.c_str() + keyOffset;
    if (memchr(keyStart, '\0', keyTotalLength - keyOffset) == nullptr) {
      throw InvalidFormat(
          "Invalid OpenCC binary dictionary (key not null-terminated)");
    }
    std::string key = keyStart;
    std::vector<std::string> values;
    for (size_t j = 0; j < numValues; j++) {
      size_t valueOffset = readField();
      if (valueOffset >= valueTotalLength) {
        throw InvalidFormat("Invalid OpenCC binary dictionary (valueOffset)");
      }
      const char* valueStart = dict->valueBuffer.c_str() + valueOffset;
      if (memchr(valueStart, '\0', valueTotalLength - valueOffset) == nullptr) {
        throw InvalidFormat(
            "Invalid OpenCC binary dictionary (value not null-terminated)");
      }
      values.push_back(valueStart);
    }
    DictEntry* entry = DictEntryFactory::New(key, values);
    dict->lexicon->Add(entry);
  }
  return dict;
}

void BinaryDict::ConstructBuffer(std::string& keyBuf,
                                 std::vector<size_t>& keyOffset,
                                 size_t& keyTotalLength, std::string& valueBuf,
                                 std::vector<size_t>& valueOffset,
                                 size_t& valueTotalLength) const {
  keyTotalLength = 0;
  valueTotalLength = 0;
  // Calculate total length
  for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
    keyTotalLength += entry->KeyLength() + 1;
    assert(entry->NumValues() != 0);
    if (entry->NumValues() == 1) {
      const auto* svEntry =
          static_cast<const SingleValueDictEntry*>(entry.get());
      valueTotalLength += svEntry->Value().length() + 1;
    } else {
      const auto* mvEntry =
          static_cast<const MultiValueDictEntry*>(entry.get());
      for (const auto& value : mvEntry->Values()) {
        valueTotalLength += value.length() + 1;
      }
    }
  }
  // Write keys and values to buffers
  keyBuf.resize(keyTotalLength, '\0');
  valueBuf.resize(valueTotalLength, '\0');
  char* pKeyBuffer = keyBuf.data();
  char* pValueBuffer = valueBuf.data();
  for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
    const std::string& key = entry->Key();
    strcpy(pKeyBuffer, key.c_str());
    keyOffset.push_back(pKeyBuffer - keyBuf.data());
    pKeyBuffer += key.length() + 1;
    if (entry->NumValues() == 1) {
      const auto* svEntry =
          static_cast<const SingleValueDictEntry*>(entry.get());
      const std::string& val = svEntry->Value();
      strcpy(pValueBuffer, val.c_str());
      valueOffset.push_back(pValueBuffer - valueBuf.data());
      pValueBuffer += val.length() + 1;
    } else {
      const auto* mvEntry =
          static_cast<const MultiValueDictEntry*>(entry.get());
      for (const auto& value : mvEntry->Values()) {
        strcpy(pValueBuffer, value.c_str());
        valueOffset.push_back(pValueBuffer - valueBuf.data());
        pValueBuffer += value.length() + 1;
      }
    }
  }
  assert(keyBuf.data() + keyTotalLength == pKeyBuffer);
  assert(valueBuf.data() + valueTotalLength == pValueBuffer);
}
