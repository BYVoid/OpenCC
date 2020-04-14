/*
 * Open Chinese Convert
 *
 * Copyright 2020 BYVoid <byvoid@byvoid.com>
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

#include "SerializedValues.hpp"
#include "Lexicon.hpp"

using namespace opencc;

size_t SerializedValues::KeyMaxLength() const {
  return 0;
}

void SerializedValues::SerializeToFile(FILE* fp) const {
  string valueBuf;
  vector<size_t> valueOffsets;
  size_t valueTotalLength = 0;
  ConstructBuffer(valueBuf, valueOffsets, valueTotalLength);
  // Number of items
  size_t numItems = lexicon->Length();
  fwrite(&numItems, sizeof(size_t), 1, fp);

  // Data
  fwrite(&valueTotalLength, sizeof(size_t), 1, fp);
  fwrite(valueBuf.c_str(), sizeof(char), valueTotalLength, fp);

  size_t valueCursor = 0;
  for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
    // Number of values
    size_t numValues = entry->NumValues();
    fwrite(&numValues, sizeof(size_t), 1, fp);
    // Values offset
    for (size_t i = 0; i < numValues; i++) {
      size_t valueOffset = valueOffsets[valueCursor++];
      fwrite(&valueOffset, sizeof(size_t), 1, fp);
    }
  }
}

std::shared_ptr<SerializedValues> SerializedValues::NewFromFile(FILE* fp) {
  std::shared_ptr<SerializedValues> dict(
      new SerializedValues(LexiconPtr(new Lexicon)));

  // Number of items
  size_t numItems;
  size_t unitsRead = fread(&numItems, sizeof(size_t), 1, fp);
  if (unitsRead != 1) {
    throw InvalidFormat("Invalid OpenCC binary dictionary (numItems)");
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
    // Value offset
    vector<std::string> values;
    for (size_t j = 0; j < numValues; j++) {
      size_t valueOffset;
      unitsRead = fread(&valueOffset, sizeof(size_t), 1, fp);
      if (unitsRead != 1) {
        throw InvalidFormat("Invalid OpenCC binary dictionary (valueOffset)");
      }
      const char* value = dict->valueBuffer.c_str() + valueOffset;
      values.push_back(value);
    }
    DictEntry* entry = DictEntryFactory::New("", values);
    dict->lexicon->Add(entry);
  }

  return dict;
}

void SerializedValues::ConstructBuffer(string& valueBuf,
                                       vector<size_t>& valueOffset,
                                       size_t& valueTotalLength) const {
  valueTotalLength = 0;
  // Calculate total length.
  for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
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
  // Write values to the buffer.
  valueBuf.resize(valueTotalLength, '\0');
  char* pValueBuffer = const_cast<char*>(valueBuf.c_str());
  for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
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
  assert(valueBuf.c_str() + valueTotalLength == pValueBuffer);
}
