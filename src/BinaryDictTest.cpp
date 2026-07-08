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
  // FIELD selects the integer field width: uint32_t for the fixed-width
  // layout written by current builds, uint64_t for the legacy layout written
  // by old 64-bit builds.
  template <typename FIELD>
  static std::string WriteCraftedBinaryDict(
      uint64_t numItems, uint64_t keyTotalLength, const std::string& keyBuffer,
      uint64_t valueTotalLength, const std::string& valueBuffer,
      const std::vector<std::tuple<uint64_t, uint64_t, std::vector<uint64_t>>>&
          items) {
    const std::string path = "crafted_binary_dict.bin";
    FILE* fp = fopen(path.c_str(), "wb");
    const auto writeField = [fp](uint64_t value) {
      FIELD field = static_cast<FIELD>(value);
      fwrite(&field, sizeof(field), 1, fp);
    };
    writeField(numItems);
    writeField(keyTotalLength);
    fwrite(keyBuffer.data(), sizeof(char), keyBuffer.size(), fp);
    writeField(valueTotalLength);
    fwrite(valueBuffer.data(), sizeof(char), valueBuffer.size(), fp);
    for (const auto& [numValues, keyOffset, valueOffsets] : items) {
      writeField(numValues);
      writeField(keyOffset);
      for (uint64_t vo : valueOffsets) {
        writeField(vo);
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

// The serialized layout must not depend on the word size of the platform:
// all integer fields are fixed 4-byte, so the total file size is fully
// determined by the lexicon content (#1412 follow-up).
TEST_F(BinaryDictTest, SerializedLayoutIsWordSizeIndependent) {
  binDict->opencc::SerializableDict::SerializeToFile(fileName);

  FILE* fp = fopen(fileName.c_str(), "rb");
  ASSERT_NE(fp, nullptr);
  fseek(fp, 0L, SEEK_END);
  const size_t fileSize = static_cast<size_t>(ftell(fp));
  fseek(fp, 0L, SEEK_SET);
  uint32_t numItems;
  ASSERT_EQ(fread(&numItems, sizeof(numItems), 1, fp), 1U);
  fclose(fp);

  EXPECT_EQ(numItems, binDict->GetLexicon()->Length());

  size_t expectedSize = 3 * sizeof(uint32_t);
  for (const auto& entry : *binDict->GetLexicon()) {
    expectedSize += entry->Key().length() + 1;   // key + NUL
    expectedSize += 2 * sizeof(uint32_t);        // numValues + keyOffset
    expectedSize += entry->NumValues() * sizeof(uint32_t); // value offsets
    for (const std::string& value : entry->Values()) {
      expectedSize += value.length() + 1;        // value + NUL
    }
  }
  EXPECT_EQ(fileSize, expectedSize);
}

// An empty dictionary serializes to exactly 12 bytes and round-trips; this is
// the only fixed-width file whose first 8 bytes are all zero, which the
// legacy-layout detection must not mistake for a 64-bit file.
TEST_F(BinaryDictTest, EmptyDictRoundTrip) {
  const std::string path = "empty_binary_dict.bin";
  const BinaryDictPtr emptyDict(new BinaryDict(LexiconPtr(new Lexicon)));
  emptyDict->opencc::SerializableDict::SerializeToFile(path);

  FILE* fp = fopen(path.c_str(), "rb");
  ASSERT_NE(fp, nullptr);
  fseek(fp, 0L, SEEK_END);
  EXPECT_EQ(ftell(fp), static_cast<long>(3 * sizeof(uint32_t)));
  fclose(fp);

  const auto deserialized = SerializableDict::NewFromFile<BinaryDict>(path);
  EXPECT_EQ(deserialized->GetLexicon()->Length(), 0U);
  std::remove(path.c_str());
}

// Test that keyTotalLength exceeding file size triggers InvalidFormat (#815).
TEST_F(BinaryDictTest, RejectsHugeKeyTotalLength) {
  std::string path = WriteCraftedBinaryDict<uint64_t>(
      1, 0x7000000000ULL, "", 0, "", {});
  EXPECT_THROW(SerializableDict::NewFromFile<BinaryDict>(path), InvalidFormat);
  std::remove(path.c_str());

  path = WriteCraftedBinaryDict<uint32_t>(1, 0x70000000ULL, "", 0, "", {});
  EXPECT_THROW(SerializableDict::NewFromFile<BinaryDict>(path), InvalidFormat);
  std::remove(path.c_str());
}

// Test that valueTotalLength exceeding file size triggers InvalidFormat (#815).
TEST_F(BinaryDictTest, RejectsHugeValueTotalLength) {
  std::string keyBuf = {'k', '\0'};
  std::string path = WriteCraftedBinaryDict<uint64_t>(
      1, 2, keyBuf, 0x7000000000ULL, "", {});
  EXPECT_THROW(SerializableDict::NewFromFile<BinaryDict>(path), InvalidFormat);
  std::remove(path.c_str());

  path = WriteCraftedBinaryDict<uint32_t>(1, 2, keyBuf, 0x70000000ULL, "", {});
  EXPECT_THROW(SerializableDict::NewFromFile<BinaryDict>(path), InvalidFormat);
  std::remove(path.c_str());
}

// Test that a keyOffset beyond keyTotalLength triggers InvalidFormat (#813).
TEST_F(BinaryDictTest, RejectsKeyOffsetOutOfBounds) {
  std::string keyBuf = {'h', 'i', '\0'};
  std::string valBuf = {'v', '\0'};
  std::string path = WriteCraftedBinaryDict<uint32_t>(
      1, 3, keyBuf, 2, valBuf, {{1, 100, {0}}});
  EXPECT_THROW(SerializableDict::NewFromFile<BinaryDict>(path), InvalidFormat);
  std::remove(path.c_str());
}

// Test that a valueOffset beyond valueTotalLength triggers InvalidFormat (#813).
TEST_F(BinaryDictTest, RejectsValueOffsetOutOfBounds) {
  std::string keyBuf = {'h', 'i', '\0'};
  std::string valBuf = {'v', '\0'};
  std::string path = WriteCraftedBinaryDict<uint32_t>(
      1, 3, keyBuf, 2, valBuf, {{1, 0, {100}}});
  EXPECT_THROW(SerializableDict::NewFromFile<BinaryDict>(path), InvalidFormat);
  std::remove(path.c_str());
}

// Sanity check: a well-formed fixed-width crafted file deserializes.
TEST_F(BinaryDictTest, AcceptsWellFormedFile) {
  std::string keyBuf = {'h', 'i', '\0'};
  std::string valBuf = {'v', '\0'};
  std::string path = WriteCraftedBinaryDict<uint32_t>(
      1, 3, keyBuf, 2, valBuf, {{1, 0, {0}}});
  const auto deserialized = SerializableDict::NewFromFile<BinaryDict>(path);
  EXPECT_EQ(deserialized->GetLexicon()->Length(), 1);
  std::remove(path.c_str());
}

// Files written by old 64-bit builds use native 8-byte size_t fields and must
// still load.
TEST_F(BinaryDictTest, AcceptsLegacy64BitLayout) {
  std::string keyBuf = {'h', 'i', '\0'};
  std::string valBuf = {'v', '\0'};
  std::string path = WriteCraftedBinaryDict<uint64_t>(
      1, 3, keyBuf, 2, valBuf, {{1, 0, {0}}});
  const auto deserialized = SerializableDict::NewFromFile<BinaryDict>(path);
  ASSERT_EQ(deserialized->GetLexicon()->Length(), 1);
  EXPECT_EQ(deserialized->GetLexicon()->At(0)->Key(), "hi");
  std::remove(path.c_str());
}

} // namespace opencc