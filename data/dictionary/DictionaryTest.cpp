/*
 * Open Chinese Convert
 *
 * Copyright 2024-2024 Carbo Kuo <byvoid@byvoid.com>
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

#include "gtest/gtest.h"

#include <unordered_map>
#include <unordered_set>

#include "src/Lexicon.hpp"
#include "src/MarisaDict.hpp"
#include "src/UTF8Util.hpp"

namespace opencc {

const char* RUNFILE_SUFFIX = ".runfiles/_main";

class DictionaryTest : public ::testing::Test,
                       public ::testing::WithParamInterface<std::string> {
protected:
  static void SetUpTestSuite() {

    std::string program_filename = ::testing::internal::GetArgvs().front();
    size_t suffix_pos = program_filename.find(RUNFILE_SUFFIX);
    ASSERT_NE(suffix_pos, std::string::npos);

    runfile_dir_ =
        program_filename.substr(0, suffix_pos + strlen(RUNFILE_SUFFIX));
  }

  static std::string runfile_dir_;
};

std::string DictionaryTest::runfile_dir_;

class DictionaryRunfilesTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    std::string program_filename = ::testing::internal::GetArgvs().front();
    size_t suffix_pos = program_filename.find(RUNFILE_SUFFIX);
    ASSERT_NE(suffix_pos, std::string::npos);
    runfile_dir_ =
        program_filename.substr(0, suffix_pos + strlen(RUNFILE_SUFFIX));
  }

  static std::string runfile_dir_;
};

std::string DictionaryRunfilesTest::runfile_dir_;

INSTANTIATE_TEST_SUITE_P(
    , DictionaryTest,
    ::testing::Values(
        "HKVariants", "HKVariantsRev", "HKVariantsRevPhrases",
        "JPShinjitaiCharacters", "JPShinjitaiPhrases", "JPVariants",
        "JPVariantsRev", "STCharacters", "STPhrases", "TSCharacters",
        "TSPhrases", "TWPhrases", "TWPhrasesRev", "TWVariants",
        "TWVariantsRev", "TWVariantsRevPhrases"),
    [](const testing::TestParamInfo<DictionaryTest::ParamType>& info) {
      return info.param;
    });

TEST_P(DictionaryTest, UniqueSortedTest) {
  const std::string dictionaryFileName =
      runfile_dir_ + "/data/dictionary/" + GetParam() + ".txt";
  FILE* fp =
      fopen(UTF8Util::GetPlatformString(dictionaryFileName).c_str(), "rb");
  ASSERT_NE(fp, nullptr);
  LexiconPtr lexicon = Lexicon::ParseLexiconFromFile(fp);
  EXPECT_TRUE(lexicon->IsUnique()) << GetParam() << " has duplicated keys.";
  EXPECT_TRUE(lexicon->IsSorted()) << GetParam() << " is not sorted.";
}

TEST_P(DictionaryTest, BinaryTest) {
  const std::string binaryDictionaryFileName =
      runfile_dir_ + "/data/dictionary/" + GetParam() + ".ocd2";
  FILE* fp_bin = fopen(
      UTF8Util::GetPlatformString(binaryDictionaryFileName).c_str(), "rb");
  ASSERT_NE(fp_bin, nullptr);
  MarisaDictPtr dict = MarisaDict::NewFromFile(fp_bin);
  ASSERT_NE(dict, nullptr);

  const std::string textDictionaryFileName =
      runfile_dir_ + "/data/dictionary/" + GetParam() + ".txt";
  FILE* fp_txt =
      fopen(UTF8Util::GetPlatformString(textDictionaryFileName).c_str(), "rb");
  ASSERT_NE(fp_txt, nullptr);
  LexiconPtr txt_lexicon = Lexicon::ParseLexiconFromFile(fp_txt);

  EXPECT_EQ(dict->GetLexicon()->Length(), txt_lexicon->Length());
}

TEST_F(DictionaryRunfilesTest, TWPhrasesReverseMapping) {
  const std::string twPhrasesFile =
      runfile_dir_ + "/data/dictionary/TWPhrases.txt";
  const std::string twPhrasesRevFile =
      runfile_dir_ + "/data/dictionary/TWPhrasesRev.txt";

  auto loadLexicon = [](const std::string& path) -> LexiconPtr {
    FILE* fp = fopen(UTF8Util::GetPlatformString(path).c_str(), "rb");
    EXPECT_NE(fp, nullptr) << path;
    if (fp == nullptr) {
      return LexiconPtr();
    }
    return Lexicon::ParseLexiconFromFile(fp);
  };

  auto buildMap = [](const LexiconPtr& lexicon)
      -> std::unordered_map<std::string, std::unordered_set<std::string>> {
    std::unordered_map<std::string, std::unordered_set<std::string>> map;
    if (!lexicon) {
      return map;
    }
    for (size_t i = 0; i < lexicon->Length(); ++i) {
      const DictEntry* entry = lexicon->At(i);
      auto& values = map[entry->Key()];
      for (const auto& value : entry->Values()) {
        values.insert(value);
      }
    }
    return map;
  };

  try {
    LexiconPtr twPhrases = loadLexicon(twPhrasesFile);
    LexiconPtr twPhrasesRev = loadLexicon(twPhrasesRevFile);
    ASSERT_NE(twPhrases, nullptr);
    ASSERT_NE(twPhrasesRev, nullptr);

    auto twMap = buildMap(twPhrases);
    auto twRevMap = buildMap(twPhrasesRev);

    for (const auto& entry : twMap) {
      const std::string& key = entry.first;
      for (const auto& value : entry.second) {
        auto it = twRevMap.find(value);
        EXPECT_TRUE(it != twRevMap.end() && it->second.count(key) > 0)
            << "Missing reverse mapping: " << key << " -> " << value;
      }
    }

    for (const auto& entry : twRevMap) {
      const std::string& key = entry.first;
      for (const auto& value : entry.second) {
        auto it = twMap.find(value);
        EXPECT_TRUE(it != twMap.end() && it->second.count(key) > 0)
            << "Missing reverse mapping: " << key << " -> " << value;
      }
    }
  } catch (const Exception& ex) {
    FAIL() << "Exception: " << ex.what();
  } catch (const std::exception& ex) {
    FAIL() << "std::exception: " << ex.what();
  } catch (...) {
    FAIL() << "Unknown exception thrown during reverse mapping check.";
  }
}

} // namespace opencc
