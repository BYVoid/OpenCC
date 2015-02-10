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
    return wordDetection.suffixes;
  }

  const vector<UTF8StringSlice>& Prefixes() const {
    return wordDetection.prefixes;
  }

  PhraseExtract wordDetection;

  const string siShi;
  const string tianGan;
};

TEST_F(PhraseExtractTest, ExtractSuffixes) {
  wordDetection.Reset();
  wordDetection.SetWordMaxLength(3);
  wordDetection.SetFullText(siShi);
  wordDetection.ExtractSuffixes();
  EXPECT_EQ(
      vector<UTF8StringSlice>({"十", "十十四是", "十四四十", "十四是十",
                               "十是十十", "十是四十", "四十", "四十是十",
                               "四十是四", "四四十是", "四是十四", "四是四十",
                               "是十十四", "是十四四", "是四十", "是四十是"}),
      Suffixes());
}

TEST_F(PhraseExtractTest, ExtractPrefixes) {
  wordDetection.Reset();
  wordDetection.SetWordMaxLength(3);
  wordDetection.SetFullText(siShi);
  wordDetection.ExtractPrefixes();
  EXPECT_EQ(
      vector<UTF8StringSlice>({"十是十十", "十四四十", "十是四十", "四是四十",
                               "四十是十", "十四是十", "四", "是十十四",
                               "四是十四", "是十四四", "四十是四", "四是四",
                               "四四十是", "是四十是", "四是", "十十四是"}),
      Prefixes());
}

TEST_F(PhraseExtractTest, ExtractWordCandidates) {
  wordDetection.Reset();
  wordDetection.SetWordMaxLength(3);
  wordDetection.SetFullText(siShi);
  wordDetection.ExtractSuffixes();
  wordDetection.CalculateFrequency();
  wordDetection.ExtractWordCandidates();
  EXPECT_EQ(vector<UTF8StringSlice>({"四", "十", "是", "四十", "是四十", "是四",
                                     "四是", "四十是", "十四", "是十", "十是",
                                     "是十四", "四是十", "四四十", "四是四",
                                     "十是四", "十是十", "十四四", "四四",
                                     "十四是", "十十四", "十十", "是十十"}),
            wordDetection.WordCandidates());
}

} // namespace opencc
