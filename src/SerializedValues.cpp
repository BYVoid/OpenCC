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

#include <cassert>
#include <cstring>

#include "Lexicon.hpp"
#include "SerializedValues.hpp"

using namespace opencc;

namespace {

template <typename INT_TYPE> INT_TYPE ReadInteger(FILE* fp) {
  INT_TYPE num;
  size_t unitsRead = fread(&num, sizeof(INT_TYPE), 1, fp);
  if (unitsRead != 1) {
    throw InvalidFormat("Invalid OpenCC binary dictionary.");
  }
  return num;
}

template <typename INT_TYPE> void WriteInteger(FILE* fp, INT_TYPE num) {
  size_t unitsWritten = fwrite(&num, sizeof(INT_TYPE), 1, fp);
  if (unitsWritten != 1) {
    throw InvalidFormat("Cannot write binary dictionary.");
  }
}

} // namespace

size_t SerializedValues::KeyMaxLength() const { return 0; }

void SerializedValues::SerializeToFile(FILE* fp) const {
  std::string valueBuf;
  std::vector<uint16_t> valueBytes;
  uint32_t valueTotalLength = 0;
  ConstructBuffer(&valueBuf, &valueBytes, &valueTotalLength);
  // Number of items
  uint32_t numItems = static_cast<uint32_t>(lexicon->Length());
  WriteInteger(fp, numItems);

  // Data
  WriteInteger(fp, valueTotalLength);
  fwrite(valueBuf.c_str(), sizeof(char), valueTotalLength, fp);

  size_t valueCursor = 0;
  for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
    // Number of values
    uint16_t numValues = static_cast<uint16_t>(entry->NumValues());
    WriteInteger(fp, numValues);
    // Values offset
    for (uint16_t i = 0; i < numValues; i++) {
      uint16_t numValueBytes = valueBytes[valueCursor++];
      WriteInteger(fp, numValueBytes);
    }
  }
}

std::shared_ptr<SerializedValues> SerializedValues::NewFromFile(FILE* fp) {
  std::shared_ptr<SerializedValues> dict(
      new SerializedValues(LexiconPtr(new Lexicon)));

  // Number of items
  uint32_t numItems = ReadInteger<uint32_t>(fp);

  // Values
  uint32_t valueTotalLength = ReadInteger<uint32_t>(fp);
  std::string valueBuffer;
  valueBuffer.resize(valueTotalLength);
  size_t unitsRead = fread(const_cast<char*>(valueBuffer.c_str()), sizeof(char),
                           valueTotalLength, fp);
  if (unitsRead != valueTotalLength) {
    throw InvalidFormat("Invalid OpenCC binary dictionary (valueBuffer)");
  }

  // Offsets
  const char* pValueBuffer = valueBuffer.c_str();
  for (uint32_t i = 0; i < numItems; i++) {
    // Number of values
    uint16_t numValues = ReadInteger<uint16_t>(fp);
    // Value offset
    std::vector<std::string> values;
    for (uint16_t j = 0; j < numValues; j++) {
      const char* value = pValueBuffer;
      uint16_t numValueBytes = ReadInteger<uint16_t>(fp);
      pValueBuffer += numValueBytes;
      values.push_back(value);
    }
    DictEntry* entry = DictEntryFactory::New("", values);
    dict->lexicon->Add(entry);
  }

  return dict;
}

void SerializedValues::ConstructBuffer(std::string* valueBuffer,
                                       std::vector<uint16_t>* valueBytes,
                                       uint32_t* valueTotalLength) const {
  *valueTotalLength = 0;
  // Calculate total length.
  for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
    assert(entry->NumValues() != 0);
    for (const auto& value : entry->Values()) {
      *valueTotalLength += static_cast<uint32_t>(value.length()) + 1;
    }
  }
  // Write values to the buffer.
  valueBuffer->resize(*valueTotalLength, '\0');
  char* pValueBuffer = const_cast<char*>(valueBuffer->c_str());
  for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
    for (const auto& value : entry->Values()) {
      strcpy(pValueBuffer, value.c_str());
      valueBytes->push_back(static_cast<uint16_t>(value.length() + 1));
      pValueBuffer += value.length() + 1;
    }
  }
  assert(valueBuffer->c_str() + *valueTotalLength == pValueBuffer);
}
