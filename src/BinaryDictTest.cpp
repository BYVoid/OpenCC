/*
 * Open Chinese Convert
 *
 * Copyright 2015-2026 Carbo Kuo and contributors
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

#include "BinaryDict.hpp"
#include "TextDictTestBase.hpp"

namespace opencc {

class BinaryDictTest : public TextDictTestBase {
protected:
  BinaryDictTest()
      : binDict(new BinaryDict(textDict->GetLexicon())), fileName("dict.bin"){};

  // Write a crafted binary file for BinaryDict with controllable fields.
  static std::string WriteMalformedBinaryDict(
      size_t numItems, size_t keyTotalLength, const std::string& keyBuffer,
      size_t valueTotalLength, const std::string& valueBuffer,
      const std::vector<std::tuple<size_t, size_t, std::vector<size_t>>>&
          items) {
    const std::string path = "malformed_binary_dict.bin";
    FILE* fp = fopen(path.c_str(), "wb");
    fwrite(&numItems, sizeof(size_t), 1, fp);
    fwrite(&keyTotalLength, sizeof(size_t), 1, fp);
    fwrite(keyBuffer.data(), sizeof(char), keyTotalLength, fp);
    fwrite(&valueTotalLength, sizeof(size_t), 1, fp);
    fwrite(valueBuffer.data(), sizeof(char), valueTotalLength, fp);
    for (const auto& [numValues, keyOffset, valueOffsets] : items) {
      fwrite(&numValues, sizeof(size_t), 1, fp);
      fwrite(&keyOffset, sizeof(size_t), 1, fp);
      for (size_t vo : valueOffsets) {
        fwrite(&vo, sizeof(size_t), 1, fp);
      }
    }
    fclose(fp);
    return path;
  }

  const BinaryDictPtr binDict;
  const std::string fileName;
};

TEST_F(BinaryDictTest, Serialization) {
  binDict->opencc::SerializableDict::SerializeToFile(fileName);
}

TEST_F(BinaryDictTest, Deserialization) {
  const BinaryDictPtr& deserialized =
      SerializableDict::NewFromFile<BinaryDict>(fileName);
  const LexiconPtr& lex1 = binDict->GetLexicon();
  const LexiconPtr& lex2 = deserialized->GetLexicon();

  // Compare every entry
  EXPECT_EQ(lex1->Length(), lex2->Length());
  for (size_t i = 0; i < lex1->Length(); i++) {
    EXPECT_EQ(lex1->At(i)->Key(), lex2->At(i)->Key());
    EXPECT_EQ(lex1->At(i)->NumValues(), lex2->At(i)->NumValues());
  }

  const TextDictPtr deserializedTextDict(new TextDict(lex2));
  TestDict(deserializedTextDict);
}

// Test that keyTotalLength exceeding file size triggers InvalidFormat (#815).
TEST_F(BinaryDictTest, RejectsHugeKeyTotalLength) {
  std::string path = WriteMalformedBinaryDict(
      1, 0x7000000000ULL, "", 0, "", {});
  EXPECT_THROW(SerializableDict::NewFromFile<BinaryDict>(path), InvalidFormat);
  std::remove(path.c_str());
}

// Test that valueTotalLength exceeding file size triggers InvalidFormat (#815).
TEST_F(BinaryDictTest, RejectsHugeValueTotalLength) {
  std::string keyBuf = {'k', '\0'};
  std::string path = WriteMalformedBinaryDict(
      1, 2, keyBuf, 0x7000000000ULL, "", {});
  EXPECT_THROW(SerializableDict::NewFromFile<BinaryDict>(path), InvalidFormat);
  std::remove(path.c_str());
}

// Test that a keyOffset beyond keyTotalLength triggers InvalidFormat (#813).
TEST_F(BinaryDictTest, RejectsKeyOffsetOutOfBounds) {
  std::string keyBuf = {'h', 'i', '\0'};
  std::string valBuf = {'v', '\0'};
  std::string path = WriteMalformedBinaryDict(
      1, 3, keyBuf, 2, valBuf, {{1, 100, {0}}});
  EXPECT_THROW(SerializableDict::NewFromFile<BinaryDict>(path), InvalidFormat);
  std::remove(path.c_str());
}

// Test that a valueOffset beyond valueTotalLength triggers InvalidFormat (#813).
TEST_F(BinaryDictTest, RejectsValueOffsetOutOfBounds) {
  std::string keyBuf = {'h', 'i', '\0'};
  std::string valBuf = {'v', '\0'};
  std::string path = WriteMalformedBinaryDict(
      1, 3, keyBuf, 2, valBuf, {{1, 0, {100}}});
  EXPECT_THROW(SerializableDict::NewFromFile<BinaryDict>(path), InvalidFormat);
  std::remove(path.c_str());
}

// Sanity check: a well-formed BinaryDict crafted file deserializes.
TEST_F(BinaryDictTest, AcceptsWellFormedFile) {
  std::string keyBuf = {'h', 'i', '\0'};
  std::string valBuf = {'v', '\0'};
  std::string path = WriteMalformedBinaryDict(
      1, 3, keyBuf, 2, valBuf, {{1, 0, {0}}});
  const auto deserialized = SerializableDict::NewFromFile<BinaryDict>(path);
  EXPECT_EQ(deserialized->GetLexicon()->Length(), 1);
  std::remove(path.c_str());
}

} // namespace opencc