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

#include "DartsDict.hpp"
#include "TestUtilsUTF8.hpp"
#include "TextDictTestBase.hpp"

namespace opencc {

class DartsDictTest : public TextDictTestBase {
protected:
  DartsDictTest()
      : dartsDict(DartsDict::NewFromDict(*textDict.get())),
        fileName("dict.ocd"){};

  // Write a crafted OCD file with controllable dartsSize.
  static std::string WriteMalformedDartsFile(size_t dartsSize) {
    const std::string path = "malformed_darts.ocd";
    FILE* fp = fopen(path.c_str(), "wb");
    const char* header = "OPENCCDARTS1";
    fwrite(header, sizeof(char), strlen(header), fp);
    fwrite(&dartsSize, sizeof(size_t), 1, fp);
    fclose(fp);
    return path;
  }

  static void CorruptDartsRootUnit(const std::string& path) {
    FILE* fp = fopen(path.c_str(), "r+b");
    const char* header = "OPENCCDARTS1";
    fseek(fp, static_cast<long>(strlen(header) + sizeof(size_t)), SEEK_SET);
    size_t invalidRoot = 0;
    fwrite(&invalidRoot, sizeof(size_t), 1, fp);
    fclose(fp);
  }

  static void CorruptFirstDartsValue(const std::string& path) {
    FILE* fp = fopen(path.c_str(), "r+b");
    const char* header = "OPENCCDARTS1";
    fseek(fp, static_cast<long>(strlen(header)), SEEK_SET);
    size_t dartsSize = 0;
    fread(&dartsSize, sizeof(size_t), 1, fp);
    const long dartsOffset =
        static_cast<long>(strlen(header) + sizeof(size_t));
    const size_t numUnits = dartsSize / sizeof(size_t);
    for (size_t i = 0; i < numUnits; i++) {
      size_t unit = 0;
      fseek(fp, dartsOffset + static_cast<long>(i * sizeof(size_t)), SEEK_SET);
      fread(&unit, sizeof(size_t), 1, fp);
      const size_t label = unit & ((1ULL << 31) | 0xFF);
      if (label > 0xFF) {
        const size_t invalidValue = (1ULL << 31) | ((1ULL << 31) - 1);
        fseek(fp, dartsOffset + static_cast<long>(i * sizeof(size_t)),
              SEEK_SET);
        fwrite(&invalidValue, sizeof(size_t), 1, fp);
        break;
      }
    }
    fclose(fp);
  }

  const DartsDictPtr dartsDict;
  const std::string fileName;
};

TEST_F(DartsDictTest, DictTest) { TestDict(dartsDict); }

TEST_F(DartsDictTest, Serialization) {
  dartsDict->opencc::SerializableDict::SerializeToFile(fileName);
}

TEST_F(DartsDictTest, Deserialization) {
  const DartsDictPtr& deserialized =
      SerializableDict::NewFromFile<DartsDict>(fileName);
  TestDict(deserialized);

  const LexiconPtr& lex1 = dartsDict->GetLexicon();
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

TEST_F(DartsDictTest, ExactMatch) {
  auto there = dartsDict->Match("積羽沉舟", 12);
  EXPECT_FALSE(there.IsNull());
  auto dictEntry = there.Get();
  EXPECT_EQ(1, dictEntry->NumValues());
  EXPECT_EQ(utf8("羣輕折軸"), dictEntry->GetDefault());

  auto nowhere = dartsDict->Match("積羽沉舟衆口鑠金", 24);
  EXPECT_TRUE(nowhere.IsNull());
}

// Test that dartsSize exceeding file size triggers InvalidFormat (#816).
TEST_F(DartsDictTest, RejectsHugeDartsSize) {
  std::string path = WriteMalformedDartsFile(0x1300000000000000ULL);
  EXPECT_THROW(SerializableDict::NewFromFile<DartsDict>(path), InvalidFormat);
  std::remove(path.c_str());
}

TEST_F(DartsDictTest, RejectsMisalignedDartsSize) {
  std::string path = WriteMalformedDartsFile(1);
  FILE* fp = fopen(path.c_str(), "ab");
  char padding = '\0';
  fwrite(&padding, sizeof(char), 1, fp);
  fclose(fp);

  EXPECT_THROW(SerializableDict::NewFromFile<DartsDict>(path), InvalidFormat);
  std::remove(path.c_str());
}

TEST_F(DartsDictTest, RejectsInvalidDartsRoot) {
  const std::string path = "invalid_darts_root.ocd";
  dartsDict->opencc::SerializableDict::SerializeToFile(path);
  CorruptDartsRootUnit(path);

  EXPECT_THROW(SerializableDict::NewFromFile<DartsDict>(path), InvalidFormat);
  std::remove(path.c_str());
}

TEST_F(DartsDictTest, RejectsInvalidDartsValue) {
  const std::string path = "invalid_darts_value.ocd";
  dartsDict->opencc::SerializableDict::SerializeToFile(path);
  CorruptFirstDartsValue(path);

  EXPECT_THROW(SerializableDict::NewFromFile<DartsDict>(path), InvalidFormat);
  std::remove(path.c_str());
}

} // namespace opencc
