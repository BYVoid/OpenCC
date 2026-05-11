/*
 * Open Chinese Convert
 *
 * Copyright 2024-2026 Carbo Kuo and contributors
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
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

namespace opencc {
namespace {

#ifndef OPENCC_DICTIONARY_TEST_FILE
#error "OPENCC_DICTIONARY_TEST_FILE must be defined"
#endif

static FILE* OpenFile(const std::string& path) {
#ifdef _MSC_VER
  return _wfopen(UTF8Util::GetPlatformString(path).c_str(), L"rb");
#else
  return fopen(UTF8Util::GetPlatformString(path).c_str(), "rb");
#endif
}

class DictionaryTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    runfiles_.reset(Runfiles::CreateForTest());
    ASSERT_NE(nullptr, runfiles_);
  }

  static std::unique_ptr<Runfiles> runfiles_;
};

std::unique_ptr<Runfiles> DictionaryTest::runfiles_;

TEST_F(DictionaryTest, UniqueSortedTest) {
  const std::string dictionaryFileName =
      runfiles_->Rlocation("_main/data/dictionary/" + dictionary);
  FILE* fp = OpenFile(dictionaryFileName);
  ASSERT_NE(fp, nullptr);
  LexiconPtr lexicon = Lexicon::ParseLexiconFromFile(fp);
  EXPECT_TRUE(lexicon->IsUnique()) << dictionary << " has duplicated keys.";
  EXPECT_TRUE(lexicon->IsSorted()) << dictionary << " is not sorted.";
}

TEST_F(DictionaryTest, BinaryTest) {
  std::string dictionary = OPENCC_DICTIONARY_TEST_FILE;
  const std::string suffix = ".txt";
  ASSERT_TRUE(dictionary.size() >= suffix.size());
  ASSERT_EQ(suffix, dictionary.substr(dictionary.size() - suffix.size()));
  dictionary.erase(dictionary.size() - suffix.size());

  const std::string binaryDictionaryFileName = runfiles_->Rlocation(
      "_main/data/dictionary/" + dictionary + ".ocd2");
  FILE* fp_bin = OpenFile(binaryDictionaryFileName);
  ASSERT_NE(fp_bin, nullptr);
  MarisaDictPtr dict = MarisaDict::NewFromFile(fp_bin);
  ASSERT_NE(dict, nullptr);

  const std::string textDictionaryFileName =
      runfiles_->Rlocation("_main/data/dictionary/" + dictionary + ".txt");
  FILE* fp_txt = OpenFile(textDictionaryFileName);
  ASSERT_NE(fp_txt, nullptr);
  LexiconPtr txt_lexicon = Lexicon::ParseLexiconFromFile(fp_txt);

  EXPECT_EQ(dict->GetLexicon()->Length(), txt_lexicon->Length());
}

} // namespace
} // namespace opencc
