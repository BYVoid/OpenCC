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

INSTANTIATE_TEST_SUITE_P(
    , DictionaryTest,
    ::testing::Values("HKVariants", "HKVariantsRevPhrases",
                      "JPShinjitaiCharacters", "JPShinjitaiPhrases",
                      "JPVariants", "STCharacters", "STPhrases", "TSCharacters",
                      "TSPhrases", "TWPhrasesIT", "TWPhrasesName",
                      "TWPhrasesOther", "TWVariants", "TWVariantsRevPhrases",
                      "TWPhrases", "TWVariantsRev", "TWPhrasesRev",
                      "HKVariantsRev", "JPVariantsRev"),
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

} // namespace opencc
