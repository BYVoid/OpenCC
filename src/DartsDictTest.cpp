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

#include <cstdint>
#include <cstring>
#include <vector>

#include "DartsDict.hpp"
#include "TestUtilsUTF8.hpp"
#include "TextDictTestBase.hpp"

namespace opencc {

class DartsDictTest : public TextDictTestBase {
protected:
  DartsDictTest()
      : dartsDict(DartsDict::NewFromDict(*textDict.get())),
        fileName("dict.ocd"){};

  // Write a crafted 32-bit-format OCD file with a controllable dartsSize.
  // A non-zero fakeFirstUnit is appended so probe[4..7] are not all zero,
  // ensuring the 32-bit detection path is taken even for malformed files.
  static std::string WriteMalformedDartsFile(uint32_t dartsSize) {
    const std::string path = "malformed_darts.ocd";
    FILE* fp = fopen(path.c_str(), "wb");
    const char* header = "OPENCCDARTS1";
    fwrite(header, sizeof(char), strlen(header), fp);
    fwrite(&dartsSize, sizeof(uint32_t), 1, fp);
    // Minimal non-zero root unit so probe[4..7] != 0 → 32-bit path taken
    uint32_t fakeFirstUnit = 0x00000400U;
    fwrite(&fakeFirstUnit, sizeof(uint32_t), 1, fp);
    fclose(fp);
    return path;
  }

  // Serialize dict as a legacy 64-bit OPENCCDARTS1 file by zero-extending
  // each 32-bit unit to 64 bits and writing dartsSize as uint64_t.
  // This reproduces files produced by old builds where id_type was size_t on
  // 64-bit platforms.
  static void SerializeAsLegacy64(const DartsDictPtr& dict,
                                  const std::string& path) {
    const std::string tmp = path + ".tmp";
    dict->opencc::SerializableDict::SerializeToFile(tmp);

    FILE* src = fopen(tmp.c_str(), "rb");
    fseek(src, 12, SEEK_SET);  // skip 12-byte magic

    uint32_t dartsSize32 = 0;
    fread(&dartsSize32, sizeof(uint32_t), 1, src);
    size_t numUnits = dartsSize32 / sizeof(uint32_t);
    std::vector<uint32_t> units32(numUnits);
    fread(units32.data(), sizeof(uint32_t), numUnits, src);
    long binaryStart = ftell(src);

    FILE* dst = fopen(path.c_str(), "wb");
    const char* header = "OPENCCDARTS1";
    fwrite(header, sizeof(char), strlen(header), dst);
    uint64_t dartsSize64 =
        static_cast<uint64_t>(numUnits) * sizeof(uint64_t);
    fwrite(&dartsSize64, sizeof(uint64_t), 1, dst);
    for (size_t i = 0; i < numUnits; ++i) {
      uint64_t unit64 = units32[i];  // zero-extend; high bits are 0
      fwrite(&unit64, sizeof(uint64_t), 1, dst);
    }

    // Copy BinaryDict section verbatim
    fseek(src, binaryStart, SEEK_SET);
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
      fwrite(buf, 1, n, dst);
    }
    fclose(src);
    fclose(dst);
    std::remove(tmp.c_str());
  }

  static void CorruptDartsRootUnit(const std::string& path) {
    FILE* fp = fopen(path.c_str(), "r+b");
    const char* header = "OPENCCDARTS1";
    // Seek past magic (12) + uint32_t dartsSize (4) to the first unit
    fseek(fp, static_cast<long>(strlen(header) + sizeof(uint32_t)), SEEK_SET);
    uint32_t invalidRoot = 0;
    fwrite(&invalidRoot, sizeof(uint32_t), 1, fp);
    fclose(fp);
  }

  static void CorruptFirstDartsValue(const std::string& path) {
    FILE* fp = fopen(path.c_str(), "r+b");
    const char* header = "OPENCCDARTS1";
    fseek(fp, static_cast<long>(strlen(header)), SEEK_SET);
    uint32_t dartsSize = 0;
    fread(&dartsSize, sizeof(uint32_t), 1, fp);
    const long dartsOffset =
        static_cast<long>(strlen(header) + sizeof(uint32_t));
    const size_t numUnits = static_cast<size_t>(dartsSize) / sizeof(uint32_t);
    for (size_t i = 0; i < numUnits; i++) {
      uint32_t unit = 0;
      fseek(fp, dartsOffset + static_cast<long>(i * sizeof(uint32_t)),
            SEEK_SET);
      fread(&unit, sizeof(uint32_t), 1, fp);
      const uint32_t label = unit & ((1U << 31) | 0xFF);
      if (label > 0xFF) {
        const uint32_t invalidValue = (1U << 31) | ((1U << 31) - 1);
        fseek(fp, dartsOffset + static_cast<long>(i * sizeof(uint32_t)),
              SEEK_SET);
        fwrite(&invalidValue, sizeof(uint32_t), 1, fp);
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

// dartsSize exceeding remaining file size must throw (#816).
TEST_F(DartsDictTest, RejectsHugeDartsSize) {
  std::string path = WriteMalformedDartsFile(0x13000000U);
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

// Verify that a legacy 64-bit OPENCCDARTS1 file (where id_type was size_t on
// a 64-bit host) loads correctly and produces identical lookup results.
TEST_F(DartsDictTest, LoadsLegacy64bitFormat) {
  const std::string path = "legacy64_dict.ocd";
  SerializeAsLegacy64(dartsDict, path);

  const DartsDictPtr& loaded = SerializableDict::NewFromFile<DartsDict>(path);
  TestDict(loaded);

  auto there = loaded->Match("積羽沉舟", 12);
  EXPECT_FALSE(there.IsNull());
  EXPECT_EQ(utf8("羣輕折軸"), there.Get()->GetDefault());

  std::remove(path.c_str());
}

// A legacy 64-bit file with a zeroed root unit must be rejected (validate).
TEST_F(DartsDictTest, RejectsInvalidLegacy64Root) {
  const std::string path = "invalid_legacy64_root.ocd";
  SerializeAsLegacy64(dartsDict, path);

  // Overwrite the root unit (magic=12, dartsSize64=8, root at offset 20)
  FILE* fp = fopen(path.c_str(), "r+b");
  fseek(fp, 12 + 8, SEEK_SET);
  uint64_t zeroUnit = 0;
  fwrite(&zeroUnit, sizeof(uint64_t), 1, fp);
  fclose(fp);

  EXPECT_THROW(SerializableDict::NewFromFile<DartsDict>(path), InvalidFormat);
  std::remove(path.c_str());
}

// Regression for 888459fb: when a legacy 64-bit OCD has >64 prefix matches,
// MatchPrefix must return the true longest match, not the 64th.
TEST_F(DartsDictTest, Legacy64PrefixSearchBeyond64Matches) {
  // Build a dict with 65 purely-nested entries: "a"×1→"v1", ..., "a"×65→"v65"
  const size_t kEntries = 65;
  LexiconPtr lex(new Lexicon);
  for (size_t i = 1; i <= kEntries; ++i) {
    lex->Add(DictEntryFactory::New(std::string(i, 'a'), "v" + std::to_string(i)));
  }
  lex->Sort();
  TextDictPtr deepText(new TextDict(lex));
  DartsDictPtr deepDict = DartsDict::NewFromDict(*deepText);

  const std::string path = "legacy64_deep.ocd";
  SerializeAsLegacy64(deepDict, path);

  const DartsDictPtr& loaded = SerializableDict::NewFromFile<DartsDict>(path);
  // All 65 entries are prefix matches; must return the longest ("v65").
  auto result = loaded->MatchPrefix(std::string(kEntries, 'a').c_str(), kEntries);
  ASSERT_FALSE(result.IsNull());
  EXPECT_EQ("v65", result.Get()->GetDefault());

  std::remove(path.c_str());
}

// SerializeToFile on a legacy-loaded dict must throw instead of crashing.
TEST_F(DartsDictTest, Legacy64SerializeToFileThrows) {
  const std::string path = "legacy64_for_serialize.ocd";
  SerializeAsLegacy64(dartsDict, path);

  const DartsDictPtr& loaded = SerializableDict::NewFromFile<DartsDict>(path);
  const std::string outPath = "legacy64_out.ocd";
  EXPECT_THROW(loaded->opencc::SerializableDict::SerializeToFile(outPath),
               InvalidFormat);

  std::remove(path.c_str());
  std::remove(outPath.c_str());
}

}  // namespace opencc
