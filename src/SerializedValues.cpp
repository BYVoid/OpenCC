/*
 * Open Chinese Convert
 *
 * Copyright 2020-2026 Carbo Kuo and contributors
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
#include <cstdint>
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

template <typename INT_TYPE>
INT_TYPE ReadIntegerFromBuffer(const char* data, size_t size, size_t* offset) {
  if (size - *offset < sizeof(INT_TYPE)) {
    throw InvalidFormat("Invalid OpenCC binary dictionary.");
  }
  INT_TYPE num;
  memcpy(&num, data + *offset, sizeof(INT_TYPE));
  *offset += sizeof(INT_TYPE);
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
  long savedOffset = ftell(fp);
  fseek(fp, 0L, SEEK_END);
  long fileEnd = ftell(fp);
  fseek(fp, savedOffset, SEEK_SET);
  size_t remainingSize =
      (fileEnd > savedOffset) ? static_cast<size_t>(fileEnd - savedOffset) : 0;

  std::string buffer;
  buffer.resize(remainingSize);
  size_t unitsRead = fread(const_cast<char*>(buffer.c_str()), sizeof(char),
                           remainingSize, fp);
  if (unitsRead != remainingSize) {
    throw InvalidFormat("Invalid OpenCC binary dictionary.");
  }

  size_t bytesRead = 0;
  return NewFromBuffer(buffer.data(), buffer.size(), &bytesRead);
}

std::shared_ptr<SerializedValues> SerializedValues::NewFromBuffer(
    const char* data, size_t size, size_t* bytesRead) {
  std::shared_ptr<SerializedValues> dict(
      new SerializedValues(LexiconPtr(new Lexicon)));
  size_t offset = 0;

  // Number of items
  uint32_t numItems = ReadIntegerFromBuffer<uint32_t>(data, size, &offset);

  // Values
  uint32_t valueTotalLength =
      ReadIntegerFromBuffer<uint32_t>(data, size, &offset);
  if (valueTotalLength > size - offset) {
    throw InvalidFormat(
        "Invalid OpenCC binary dictionary (valueTotalLength exceeds file size)");
  }
  std::string valueBuffer;
  valueBuffer.resize(valueTotalLength);
  memcpy(valueBuffer.data(), data + offset, valueTotalLength);
  offset += valueTotalLength;

  // Offsets
  const char* pValueBuffer = valueBuffer.c_str();
  const char* pValueBufferEnd = pValueBuffer + valueTotalLength;
  for (uint32_t i = 0; i < numItems; i++) {
    // Number of values
    uint16_t numValues = ReadIntegerFromBuffer<uint16_t>(data, size, &offset);
    // Value offset
    std::vector<std::string> values;
    for (uint16_t j = 0; j < numValues; j++) {
      uint16_t numValueBytes =
          ReadIntegerFromBuffer<uint16_t>(data, size, &offset);
      if (numValueBytes == 0 || pValueBuffer + numValueBytes > pValueBufferEnd) {
        throw InvalidFormat(
            "Invalid OpenCC binary dictionary (value offset out of bounds)");
      }
      if (pValueBuffer[numValueBytes - 1] != '\0') {
        throw InvalidFormat(
            "Invalid OpenCC binary dictionary (value not null-terminated)");
      }
      const char* value = pValueBuffer;
      pValueBuffer += numValueBytes;
      values.push_back(value);
    }
    DictEntry* entry = DictEntryFactory::New("", values);
    dict->lexicon->Add(entry);
  }

  if (bytesRead != nullptr) {
    *bytesRead = offset;
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
  char* pValueBuffer = valueBuffer->data();
  for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
    for (const auto& value : entry->Values()) {
      strcpy(pValueBuffer, value.c_str());
      valueBytes->push_back(static_cast<uint16_t>(value.length() + 1));
      pValueBuffer += value.length() + 1;
    }
  }
  assert(valueBuffer->data() + *valueTotalLength == pValueBuffer);
}
