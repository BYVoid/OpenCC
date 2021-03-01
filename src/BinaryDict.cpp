/*
 * Open Chinese Convert
 *
 * Copyright 2010-2020 Carbo Kuo <byvoid@byvoid.com>
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

void BinaryDict::SerializeToFile(FILE* fp) const {
  std::string keyBuf, valueBuf;
  std::vector<size_t> keyOffsets, valueOffsets;
  size_t keyTotalLength = 0, valueTotalLength = 0;
  ConstructBuffer(keyBuf, keyOffsets, keyTotalLength, valueBuf, valueOffsets,
                  valueTotalLength);
  // Number of items
  size_t numItems = lexicon->Length();
  fwrite(&numItems, sizeof(size_t), 1, fp);

  // Data
  fwrite(&keyTotalLength, sizeof(size_t), 1, fp);
  fwrite(keyBuf.c_str(), sizeof(char), keyTotalLength, fp);
  fwrite(&valueTotalLength, sizeof(size_t), 1, fp);
  fwrite(valueBuf.c_str(), sizeof(char), valueTotalLength, fp);

  size_t keyCursor = 0, valueCursor = 0;
  for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
    // Number of values
    size_t numValues = entry->NumValues();
    fwrite(&numValues, sizeof(size_t), 1, fp);
    // Key offset
    size_t keyOffset = keyOffsets[keyCursor++];
    fwrite(&keyOffset, sizeof(size_t), 1, fp);
    // Values offset
    for (size_t i = 0; i < numValues; i++) {
      size_t valueOffset = valueOffsets[valueCursor++];
      fwrite(&valueOffset, sizeof(size_t), 1, fp);
    }
  }
  assert(keyCursor == numItems);
}

BinaryDictPtr BinaryDict::NewFromFile(FILE* fp) {
  size_t offsetBound, savedOffset;
  savedOffset = ftell(fp);
  fseek(fp, 0L, SEEK_END);
  offsetBound = ftell(fp) - savedOffset;
  fseek(fp, savedOffset, SEEK_SET);

  BinaryDictPtr dict(new BinaryDict(LexiconPtr(new Lexicon)));

  // Number of items
  size_t numItems;
  size_t unitsRead = fread(&numItems, sizeof(size_t), 1, fp);
  if (unitsRead != 1) {
    throw InvalidFormat("Invalid OpenCC binary dictionary (numItems)");
  }

  // Keys
  size_t keyTotalLength;
  unitsRead = fread(&keyTotalLength, sizeof(size_t), 1, fp);
  if (unitsRead != 1) {
    throw InvalidFormat("Invalid OpenCC binary dictionary (keyTotalLength)");
  }
  dict->keyBuffer.resize(keyTotalLength);
  unitsRead = fread(const_cast<char*>(dict->keyBuffer.c_str()), sizeof(char),
                    keyTotalLength, fp);
  if (unitsRead != keyTotalLength) {
    throw InvalidFormat("Invalid OpenCC binary dictionary (keyBuffer)");
  }

  // Values
  size_t valueTotalLength;
  unitsRead = fread(&valueTotalLength, sizeof(size_t), 1, fp);
  if (unitsRead != 1) {
    throw InvalidFormat("Invalid OpenCC binary dictionary (valueTotalLength)");
  }
  dict->valueBuffer.resize(valueTotalLength);
  unitsRead = fread(const_cast<char*>(dict->valueBuffer.c_str()), sizeof(char),
                    valueTotalLength, fp);
  if (unitsRead != valueTotalLength) {
    throw InvalidFormat("Invalid OpenCC binary dictionary (valueBuffer)");
  }

  // Offsets
  for (size_t i = 0; i < numItems; i++) {
    // Number of values
    size_t numValues;
    unitsRead = fread(&numValues, sizeof(size_t), 1, fp);
    if (unitsRead != 1) {
      throw InvalidFormat("Invalid OpenCC binary dictionary (numValues)");
    }
    // Key offset
    size_t keyOffset;
    unitsRead = fread(&keyOffset, sizeof(size_t), 1, fp);
    if (unitsRead != 1 || keyOffset >= offsetBound) {
      throw InvalidFormat("Invalid OpenCC binary dictionary (keyOffset)");
    }
    std::string key = dict->keyBuffer.c_str() + keyOffset;
    // Value offset
    std::vector<std::string> values;
    for (size_t j = 0; j < numValues; j++) {
      size_t valueOffset;
      unitsRead = fread(&valueOffset, sizeof(size_t), 1, fp);
      if (unitsRead != 1 || valueOffset >= offsetBound) {
        throw InvalidFormat("Invalid OpenCC binary dictionary (valueOffset)");
      }
      const char* value = dict->valueBuffer.c_str() + valueOffset;
      values.push_back(value);
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
  char* pKeyBuffer = const_cast<char*>(keyBuf.c_str());
  char* pValueBuffer = const_cast<char*>(valueBuf.c_str());
  for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
    strcpy(pKeyBuffer, entry->Key().c_str());
    keyOffset.push_back(pKeyBuffer - keyBuf.c_str());
    pKeyBuffer += entry->KeyLength() + 1;
    if (entry->NumValues() == 1) {
      const auto* svEntry =
          static_cast<const SingleValueDictEntry*>(entry.get());
      strcpy(pValueBuffer, svEntry->Value().c_str());
      valueOffset.push_back(pValueBuffer - valueBuf.c_str());
      pValueBuffer += svEntry->Value().length() + 1;
    } else {
      const auto* mvEntry =
          static_cast<const MultiValueDictEntry*>(entry.get());
      for (const auto& value : mvEntry->Values()) {
        strcpy(pValueBuffer, value.c_str());
        valueOffset.push_back(pValueBuffer - valueBuf.c_str());
        pValueBuffer += value.length() + 1;
      }
    }
  }
  assert(keyBuf.c_str() + keyTotalLength == pKeyBuffer);
  assert(valueBuf.c_str() + valueTotalLength == pValueBuffer);
}
