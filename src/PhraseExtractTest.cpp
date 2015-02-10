/*
 * Open Chinese Convert
 *
 * Copyright 2015 BYVoid <byvoid@byvoid.com>
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

#include "PhraseExtract.hpp"
#include "TestUtils.hpp"

namespace opencc {

class PhraseExtractTest : public ::testing::Test {
protected:
  PhraseExtractTest()
      : siShi(utf8("四是四十是十十四是十四四十是四十")),
        tianGan(utf8("甲乙丙丁戊己庚辛壬癸")) {}

  const vector<UTF8StringSlice>& Suffixes() const {
    return phraseExtract.suffixes;
  }

  const vector<UTF8StringSlice>& Prefixes() const {
    return phraseExtract.prefixes;
  }

  const std::unordered_map<UTF8StringSlice, size_t, UTF8StringSlice::Hasher>&
  Frequencies() const {
    return phraseExtract.frequencies;
  }

  PhraseExtract phraseExtract;

  const string siShi;
  const string tianGan;
};

TEST_F(PhraseExtractTest, ExtractSuffixes) {
  phraseExtract.Reset();
  phraseExtract.SetWordMaxLength(3);
  phraseExtract.SetFullText(siShi);
  phraseExtract.ExtractSuffixes();
  EXPECT_EQ(
      vector<UTF8StringSlice>({"十", "十十四是", "十四四十", "十四是十",
                               "十是十十", "十是四十", "四十", "四十是十",
                               "四十是四", "四四十是", "四是十四", "四是四十",
                               "是十十四", "是十四四", "是四十", "是四十是"}),
      Suffixes());
}

TEST_F(PhraseExtractTest, ExtractPrefixes) {
  phraseExtract.Reset();
  phraseExtract.SetWordMaxLength(3);
  phraseExtract.SetFullText(siShi);
  phraseExtract.ExtractPrefixes();
  EXPECT_EQ(
      vector<UTF8StringSlice>({"十是十十", "十四四十", "十是四十", "四是四十",
                               "四十是十", "十四是十", "四", "是十十四",
                               "四是十四", "是十四四", "四十是四", "四是四",
                               "四四十是", "是四十是", "四是", "十十四是"}),
      Prefixes());
}

TEST_F(PhraseExtractTest, CalculateFrequency) {
  phraseExtract.Reset();
  phraseExtract.SetWordMaxLength(3);
  phraseExtract.SetFullText(siShi);
  phraseExtract.CalculateFrequency();
  EXPECT_EQ(23, Frequencies().size());
  EXPECT_EQ(6, phraseExtract.Frequency("四"));
  EXPECT_EQ(6, phraseExtract.Frequency("十"));
  EXPECT_EQ(4, phraseExtract.Frequency("是"));
  EXPECT_EQ(3, phraseExtract.Frequency("四十"));
  EXPECT_EQ(2, phraseExtract.Frequency("是四十"));
  EXPECT_EQ(2, phraseExtract.Frequency("是四"));
  EXPECT_EQ(2, phraseExtract.Frequency("四是"));
}

TEST_F(PhraseExtractTest, ExtractWordCandidates) {
  phraseExtract.Reset();
  phraseExtract.SetWordMaxLength(3);
  phraseExtract.SetFullText(siShi);
  phraseExtract.ExtractWordCandidates();
  EXPECT_EQ(vector<UTF8StringSlice>({"十", "四", "是", "四十", "十四", "十是",
                                     "四十是", "四是", "是十", "是四", "是四十",
                                     "十十", "十十四", "十四四", "十四是",
                                     "十是十", "十是四", "四四", "四四十",
                                     "四是十", "四是四", "是十十", "是十四"}),
            phraseExtract.WordCandidates());
}

} // namespace opencc
