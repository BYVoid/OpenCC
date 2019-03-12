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

#include <cmath>

#include "PhraseExtract.hpp"
#include "TestUtils.hpp"

namespace opencc {

typedef PhraseExtract::UTF8StringSlice8Bit UTF8StringSlice8Bit;

class PhraseExtractTest : public ::testing::Test {
protected:
  PhraseExtractTest()
      : siShi(utf8("四是四十是十十四是十四四十是四十")),
        punctuation(utf8("一.二.三")) {}

  const vector<UTF8StringSlice8Bit>& Suffixes() const {
    return phraseExtract.suffixes;
  }

  const vector<UTF8StringSlice8Bit>& Prefixes() const {
    return phraseExtract.prefixes;
  }

  PhraseExtract phraseExtract;

  const string siShi;
  const string punctuation;
};

TEST_F(PhraseExtractTest, ExtractSuffixes) {
  phraseExtract.Reset();
  phraseExtract.SetWordMinLength(1);
  phraseExtract.SetWordMaxLength(3);
  phraseExtract.SetFullText(siShi);
  phraseExtract.ExtractSuffixes();
  EXPECT_EQ(
      vector<UTF8StringSlice8Bit>(
          {"十", "十十四是", "十四四十", "十四是十", "十是十十", "十是四十",
           "四十", "四十是十", "四十是四", "四四十是", "四是十四", "四是四十",
           "是十十四", "是十四四", "是四十", "是四十是"}),
      Suffixes());
}

TEST_F(PhraseExtractTest, ExtractPrefixes) {
  phraseExtract.Reset();
  phraseExtract.SetWordMinLength(1);
  phraseExtract.SetWordMaxLength(3);
  phraseExtract.SetFullText(siShi);
  phraseExtract.ExtractPrefixes();
  EXPECT_EQ(
      vector<UTF8StringSlice8Bit>(
          {"十是十十", "十四四十", "十是四十", "四是四十", "四十是十",
           "十四是十", "四", "是十十四", "四是十四", "是十四四", "四十是四",
           "四是四", "四四十是", "是四十是", "四是", "十十四是"}),
      Prefixes());
}

TEST_F(PhraseExtractTest, CalculateFrequency) {
  phraseExtract.Reset();
  phraseExtract.SetWordMinLength(1);
  phraseExtract.SetWordMaxLength(3);
  phraseExtract.SetFullText(siShi);
  phraseExtract.CalculateFrequency();
  EXPECT_EQ(6, phraseExtract.Frequency("四"));
  EXPECT_EQ(6, phraseExtract.Frequency("十"));
  EXPECT_EQ(4, phraseExtract.Frequency("是"));
  EXPECT_EQ(3, phraseExtract.Frequency("四十"));
  EXPECT_EQ(2, phraseExtract.Frequency("是四十"));
  EXPECT_EQ(2, phraseExtract.Frequency("是四"));
  EXPECT_EQ(2, phraseExtract.Frequency("四是"));
  EXPECT_DOUBLE_EQ(-2.0149030205422647, phraseExtract.LogProbability("四"));
  EXPECT_DOUBLE_EQ(-2.0149030205422647, phraseExtract.LogProbability("十"));
  EXPECT_DOUBLE_EQ(-2.4203681286504288, phraseExtract.LogProbability("是"));
  EXPECT_DOUBLE_EQ(-2.7080502011022096, phraseExtract.LogProbability("四十"));
  EXPECT_DOUBLE_EQ(-3.8066624897703196, phraseExtract.LogProbability("是十十"));
}

TEST_F(PhraseExtractTest, ExtractWordCandidates) {
  phraseExtract.Reset();
  phraseExtract.SetWordMinLength(1);
  phraseExtract.SetWordMaxLength(3);
  phraseExtract.SetFullText(siShi);
  phraseExtract.ExtractWordCandidates();
  EXPECT_EQ(
      vector<UTF8StringSlice8Bit>(
          {"十", "四", "是", "四十", "十四", "十是", "四十是", "四是", "是十",
           "是四", "是四十", "十十", "十十四", "十四四", "十四是", "十是十",
           "十是四", "四四", "四四十", "四是十", "四是四", "是十十", "是十四"}),
      phraseExtract.WordCandidates());
}

TEST_F(PhraseExtractTest, CalculateCohesions) {
  phraseExtract.Reset();
  phraseExtract.SetWordMinLength(1);
  phraseExtract.SetWordMaxLength(3);
  phraseExtract.SetFullText(siShi);
  phraseExtract.CalculateCohesions();
  EXPECT_DOUBLE_EQ(INFINITY, phraseExtract.Cohesion("四"));
  EXPECT_DOUBLE_EQ(1.3217558399823193, phraseExtract.Cohesion("四十"));
  EXPECT_DOUBLE_EQ(0.91629073187415511, phraseExtract.Cohesion("十四"));
  EXPECT_DOUBLE_EQ(1.3217558399823193, phraseExtract.Cohesion("十是"));
  EXPECT_DOUBLE_EQ(1.3217558399823193, phraseExtract.Cohesion("四是四"));
  EXPECT_DOUBLE_EQ(1.3217558399823193, phraseExtract.Cohesion("十是十"));
}

TEST_F(PhraseExtractTest, CalculateSuffixEntropy) {
  phraseExtract.Reset();
  phraseExtract.SetWordMinLength(1);
  phraseExtract.SetWordMaxLength(3);
  phraseExtract.SetFullText(siShi);
  phraseExtract.CalculateSuffixEntropy();
  EXPECT_DOUBLE_EQ(1.0549201679861442, phraseExtract.SuffixEntropy("十"));
  EXPECT_DOUBLE_EQ(1.0114042647073518, phraseExtract.SuffixEntropy("四"));
  EXPECT_DOUBLE_EQ(0.69314718055994529, phraseExtract.SuffixEntropy("十四"));
  EXPECT_DOUBLE_EQ(0.69314718055994529, phraseExtract.SuffixEntropy("十是"));
  EXPECT_DOUBLE_EQ(0, phraseExtract.SuffixEntropy("四十"));
  EXPECT_DOUBLE_EQ(0, phraseExtract.SuffixEntropy("四是四"));
  EXPECT_DOUBLE_EQ(0, phraseExtract.SuffixEntropy("十是十"));
}

TEST_F(PhraseExtractTest, CalculatePrefixEntropy) {
  phraseExtract.Reset();
  phraseExtract.SetWordMinLength(1);
  phraseExtract.SetWordMaxLength(3);
  phraseExtract.SetFullText(siShi);
  phraseExtract.CalculatePrefixEntropy();
  EXPECT_DOUBLE_EQ(1.0114042647073516, phraseExtract.PrefixEntropy("十"));
  EXPECT_DOUBLE_EQ(1.0549201679861442, phraseExtract.PrefixEntropy("四"));
  EXPECT_DOUBLE_EQ(0.69314718055994529, phraseExtract.PrefixEntropy("十四"));
  EXPECT_DOUBLE_EQ(0, phraseExtract.PrefixEntropy("十是"));
  EXPECT_DOUBLE_EQ(0.63651416829481278, phraseExtract.PrefixEntropy("四十"));
  EXPECT_DOUBLE_EQ(0, phraseExtract.PrefixEntropy("四是四"));
  EXPECT_DOUBLE_EQ(0, phraseExtract.PrefixEntropy("十是十"));
}

TEST_F(PhraseExtractTest, SelectWords) {
  phraseExtract.Reset();
  phraseExtract.SetWordMinLength(1);
  phraseExtract.SetWordMaxLength(3);
  phraseExtract.SetFullText(siShi);
  phraseExtract.SetPostCalculationFilter(
      [](const PhraseExtract& phraseExtract, const UTF8StringSlice8Bit& word) {
        return phraseExtract.Frequency(word) == 1;
      });
  phraseExtract.SelectWords();
  EXPECT_EQ(
      vector<UTF8StringSlice8Bit>({"十", "四", "是", "四十", "十四", "十是",
                                   "四十是", "四是", "是十", "是四", "是四十"}),
      phraseExtract.Words());
}

TEST_F(PhraseExtractTest, Punctuation) {
  phraseExtract.Reset();
  phraseExtract.SetWordMinLength(1);
  phraseExtract.SetWordMaxLength(2);
  phraseExtract.SetFullText(punctuation);
  phraseExtract.ExtractPrefixes();
  EXPECT_EQ(
      vector<UTF8StringSlice8Bit>({"一.", ".二.", "一", "二.三", "一.二"}),
      Prefixes());
}

} // namespace opencc
