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

TEST_P(DictionaryTest, DictTest) {
  const std::string dictionaryFileName =
      runfile_dir_ + "/data/dictionary/" + GetParam();
  FILE* fp =
      fopen(UTF8Util::GetPlatformString(dictionaryFileName).c_str(), "rb");
  ASSERT_NE(fp, nullptr);
  LexiconPtr lexicon = Lexicon::ParseLexiconFromFile(fp);
  EXPECT_TRUE(lexicon->IsUnique()) << GetParam() << " has duplicated keys.";
  EXPECT_TRUE(lexicon->IsSorted()) << GetParam() << " is not sorted.";
}

INSTANTIATE_TEST_SUITE_P(
    Dictionary, DictionaryTest,
    ::testing::Values("HKVariants.txt", "HKVariantsRevPhrases.txt",
                      "JPShinjitaiCharacters.txt", "JPShinjitaiPhrases.txt",
                      "JPVariants.txt", "STCharacters.txt", "STPhrases.txt",
                      "TSCharacters.txt", "TSPhrases.txt", "TWPhrasesIT.txt",
                      "TWPhrasesName.txt", "TWPhrasesOther.txt",
                      "TWVariants.txt", "TWVariantsRevPhrases.txt"));

} // namespace opencc
