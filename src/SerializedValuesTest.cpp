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

#include <cstdio>
#include <cstring>

#include "SerializedValues.hpp"
#include "TextDictTestBase.hpp"

namespace opencc {

class SerializedValuesTest : public TextDictTestBase {
protected:
  SerializedValuesTest()
      : binDict(new SerializedValues(textDict->GetLexicon())),
        fileName("dict.bin"){};

  // Write a crafted binary file for testing malformed input handling.
  // Format: uint32 numItems, uint32 valueTotalLength, char[] valueBuffer,
  //         then per item: uint16 numValues, per value: uint16 numValueBytes
  static std::string WriteMalformedFile(uint32_t numItems,
                                        uint32_t valueTotalLength,
                                        const std::string& valueBuffer,
                                        const std::vector<std::vector<uint16_t>>&
                                            itemValueBytes) {
    const std::string path = "malformed_test.bin";
    FILE* fp = fopen(path.c_str(), "wb");
    fwrite(&numItems, sizeof(uint32_t), 1, fp);
    fwrite(&valueTotalLength, sizeof(uint32_t), 1, fp);
    fwrite(valueBuffer.data(), sizeof(char), valueTotalLength, fp);
    for (const auto& item : itemValueBytes) {
      uint16_t numValues = static_cast<uint16_t>(item.size());
      fwrite(&numValues, sizeof(uint16_t), 1, fp);
      for (uint16_t nb : item) {
        fwrite(&nb, sizeof(uint16_t), 1, fp);
      }
    }
    fclose(fp);
    return path;
  }

  const std::shared_ptr<SerializedValues> binDict;
  const std::string fileName;
};

TEST_F(SerializedValuesTest, Serialization) {
  binDict->opencc::SerializableDict::SerializeToFile(fileName);
}

TEST_F(SerializedValuesTest, Deserialization) {
  const std::shared_ptr<SerializedValues>& deserialized =
      SerializableDict::NewFromFile<SerializedValues>(fileName);
  const LexiconPtr& lex1 = binDict->GetLexicon();
  const LexiconPtr& lex2 = deserialized->GetLexicon();

  // Compare every entry
  EXPECT_EQ(lex1->Length(), lex2->Length());
  for (size_t i = 0; i < lex1->Length(); i++) {
    EXPECT_EQ(lex1->At(i)->NumValues(), lex2->At(i)->NumValues());
  }
}

// Test that numValueBytes exceeding the value buffer triggers InvalidFormat.
TEST_F(SerializedValuesTest, RejectsValueOffsetOutOfBounds) {
  // Value buffer has 4 bytes: "ab\0\0", but we claim the single value is 10
  // bytes, which would read past the buffer end.
  std::string valueBuf = {'a', 'b', '\0', '\0'};
  std::string path =
      WriteMalformedFile(1, 4, valueBuf, {{10}});
  EXPECT_THROW(SerializableDict::NewFromFile<SerializedValues>(path),
               InvalidFormat);
  std::remove(path.c_str());
}

// Test that a value whose region is not null-terminated triggers InvalidFormat.
TEST_F(SerializedValuesTest, RejectsValueNotNullTerminated) {
  // Value buffer has 4 bytes with no null terminator in the claimed range.
  std::string valueBuf = {'a', 'b', 'c', '\0'};
  // Claim numValueBytes=3, so bytes [0..2] = "abc" with no null at index 2.
  std::string path =
      WriteMalformedFile(1, 4, valueBuf, {{3}});
  EXPECT_THROW(SerializableDict::NewFromFile<SerializedValues>(path),
               InvalidFormat);
  std::remove(path.c_str());
}

// Test that numValueBytes of zero triggers InvalidFormat.
TEST_F(SerializedValuesTest, RejectsZeroValueBytes) {
  std::string valueBuf = {'a', '\0'};
  std::string path =
      WriteMalformedFile(1, 2, valueBuf, {{0}});
  EXPECT_THROW(SerializableDict::NewFromFile<SerializedValues>(path),
               InvalidFormat);
  std::remove(path.c_str());
}

// Test that cumulative offsets exceeding buffer length are caught.
TEST_F(SerializedValuesTest, RejectsCumulativeOffsetOverflow) {
  // Buffer has 6 bytes: two null-terminated strings "ab\0" and "cd\0".
  // First value correctly uses 3 bytes, second claims 5 which overflows.
  std::string valueBuf = {'a', 'b', '\0', 'c', 'd', '\0'};
  std::string path =
      WriteMalformedFile(1, 6, valueBuf, {{3, 5}});
  EXPECT_THROW(SerializableDict::NewFromFile<SerializedValues>(path),
               InvalidFormat);
  std::remove(path.c_str());
}

// Sanity check: a well-formed crafted file deserializes successfully.
TEST_F(SerializedValuesTest, AcceptsWellFormedFile) {
  // "ab\0" (3 bytes) + "cd\0" (3 bytes) = 6 byte buffer, two items each with
  // one value.
  std::string valueBuf = {'a', 'b', '\0', 'c', 'd', '\0'};
  std::string path =
      WriteMalformedFile(2, 6, valueBuf, {{3}, {3}});
  const auto deserialized =
      SerializableDict::NewFromFile<SerializedValues>(path);
  EXPECT_EQ(deserialized->GetLexicon()->Length(), 2);
  std::remove(path.c_str());
}

} // namespace opencc
