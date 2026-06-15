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

#include "UTF8Util.hpp"
#include "TestUtils.hpp"

namespace opencc {

class UTF8UtilTest : public ::testing::Test {
protected:
  UTF8UtilTest() : text("東菄鶇䍶𠍀倲𩜍𢘐"), length(strlen(text)){};
  const char* text;
  const size_t length;
};

TEST_F(UTF8UtilTest, NextCharLength) {
  EXPECT_EQ(3, UTF8Util::NextCharLength(text));
  EXPECT_EQ(3, UTF8Util::NextCharLength(text + 3));
  EXPECT_EQ(3, UTF8Util::NextCharLength(text + 6));
  EXPECT_EQ(3, UTF8Util::NextCharLength(text + 9));
  EXPECT_EQ(4, UTF8Util::NextCharLength(text + 12));
  EXPECT_EQ(3, UTF8Util::NextCharLength(text + 16));
  EXPECT_EQ(4, UTF8Util::NextCharLength(text + 19));
  EXPECT_EQ(4, UTF8Util::NextCharLength(text + 23));
  EXPECT_THROW(EXPECT_EQ(3, UTF8Util::NextCharLength(text + 1)), InvalidUTF8);
  EXPECT_THROW(EXPECT_EQ(3, UTF8Util::NextCharLength(text + 2)), InvalidUTF8);
}

TEST_F(UTF8UtilTest, PrevCharLength) {
  EXPECT_EQ(4, UTF8Util::PrevCharLength(text + length));
  EXPECT_EQ(4, UTF8Util::PrevCharLength(text + length - 4));
  EXPECT_EQ(3, UTF8Util::PrevCharLength(text + length - 8));
  EXPECT_THROW(EXPECT_EQ(3, UTF8Util::PrevCharLength(text + 1)), InvalidUTF8);
}

TEST(UTF8UtilASCIITest, PrevCharLengthASCII) {
  // Heap-allocated ASCII string to catch issue #794 (OOB read with ASCII input)
  const std::string asciiStr = "abc";
  EXPECT_EQ(1,
            UTF8Util::PrevCharLength(asciiStr.c_str() + asciiStr.size()));
  EXPECT_EQ(1, UTF8Util::PrevCharLength(asciiStr.c_str() + 2));
  EXPECT_EQ(1, UTF8Util::PrevCharLength(asciiStr.c_str() + 1));
}

TEST(UTF8UtilTruncatedTest, LengthTruncatedSequence) {
  // Length() must throw InvalidUTF8 for truncated sequences instead of
  // reading past the null terminator (issue #799 fix).
  EXPECT_THROW(UTF8Util::Length("\xE0"), InvalidUTF8);   // incomplete 3-byte sequence
  EXPECT_THROW(UTF8Util::Length("\xF0"), InvalidUTF8);   // incomplete 4-byte sequence
  EXPECT_THROW(UTF8Util::Length("\xE0\xBF"), InvalidUTF8); // still incomplete 3-byte
}

TEST_F(UTF8UtilTest, Length) {
  EXPECT_EQ(0, UTF8Util::Length(""));
  EXPECT_EQ(8, UTF8Util::Length(text));
}

TEST_F(UTF8UtilTest, NotShorterThan) {
  EXPECT_TRUE(UTF8Util::NotShorterThan(text, 0));
  EXPECT_TRUE(UTF8Util::NotShorterThan(text, length));
  EXPECT_FALSE(UTF8Util::NotShorterThan(text, length + 1));
}

TEST_F(UTF8UtilTest, TruncateUTF8) {
  EXPECT_EQ("", UTF8Util::TruncateUTF8(text, 0));
  EXPECT_EQ("", UTF8Util::TruncateUTF8(text, 1));
  EXPECT_EQ("東", UTF8Util::TruncateUTF8(text, 3));
  EXPECT_EQ("東", UTF8Util::TruncateUTF8(text, 4));
  EXPECT_EQ("東菄鶇䍶𠍀", UTF8Util::TruncateUTF8(text, 16));
  EXPECT_EQ(text, UTF8Util::TruncateUTF8(text, length));
  EXPECT_EQ(text, UTF8Util::TruncateUTF8(text, length + 1));
}

TEST(UTF8UtilIDSTest, IdeographicDescriptionSequenceLength) {
  const std::string sequence = "⿰钅只只";
  EXPECT_EQ(strlen("⿰钅只"),
            UTF8Util::NextIdeographicDescriptionSequenceLength(
                sequence.c_str(), sequence.length()));
}

TEST(UTF8UtilIDSTest, NestedIdeographicDescriptionSequenceLength) {
  const std::string sequence = "⿱艹⿰钅只干";
  EXPECT_EQ(strlen("⿱艹⿰钅只"),
            UTF8Util::NextIdeographicDescriptionSequenceLength(
                sequence.c_str(), sequence.length()));
}

TEST(UTF8UtilIDSTest, ComplexIdeographicDescriptionSequenceLength) {
  const std::string sequence = "⿺⻍⿳穴⿲月⿱⿲幺言幺⿲長馬長刂心長";
  EXPECT_EQ(strlen("⿺⻍⿳穴⿲月⿱⿲幺言幺⿲長馬長刂心"),
            UTF8Util::NextIdeographicDescriptionSequenceLength(
                sequence.c_str(), sequence.length()));
}

TEST(UTF8UtilIDSTest, OperatorOnlyIdeographicDescriptionSequenceReturnsZero) {
  const std::string sequence = "⿰⿱⿲⿳⿴⿵⿶⿷⿸⿹⿺⿻⿼⿽⿾⿿";
  EXPECT_EQ(0, UTF8Util::NextIdeographicDescriptionSequenceLength(
                   sequence.c_str(), sequence.length()));
}

TEST(UTF8UtilIDSTest, IsolatedIdeographicDescriptionCharactersAreNotSequences) {
  const std::vector<std::string> operators = {
      "⿰", "⿱", "⿲", "⿳", "⿴", "⿵", "⿶", "⿷",
      "⿸", "⿹", "⿺", "⿻", "⿼", "⿽", "⿾", "⿿"};
  for (const std::string& op : operators) {
    EXPECT_EQ(0, UTF8Util::NextIdeographicDescriptionSequenceLength(
                     op.c_str(), op.length()))
        << op;
  }
}

TEST(UTF8UtilIDSTest, IncompleteIdeographicDescriptionSequenceReturnsZero) {
  const std::string sequence = "⿰钅";
  EXPECT_EQ(0, UTF8Util::NextIdeographicDescriptionSequenceLength(
                   sequence.c_str(), sequence.length()));
}

TEST(UTF8UtilIDSTest, OverlyDeepIdeographicDescriptionSequenceReturnsZero) {
  const std::string sequence = "⿰⿰⿰⿰⿰⿰⿰⿰⿰⿰⿰⿰⿰⿰⿰⿰⿰木木木木木木木木木木木木木木木木木木";
  EXPECT_EQ(0, UTF8Util::NextIdeographicDescriptionSequenceLength(
                   sequence.c_str(), sequence.length()));
}

TEST(UTF8UtilIDSTest, OverlyLongIdeographicDescriptionSequenceReturnsZero) {
  std::string sequence;
  for (size_t i = 0; i < 64; i++) {
    sequence += "⿰";
  }
  for (size_t i = 0; i < 65; i++) {
    sequence += "木";
  }
  EXPECT_EQ(0, UTF8Util::NextIdeographicDescriptionSequenceLength(
                   sequence.c_str(), sequence.length()));
}

TEST(UTF8UtilVariationSelectorTest, ContainsVariationSelector) {
  const std::string bmpVariationSelector = std::string("禰") + "\xEF\xB8\x80";
  const std::string supplementaryVariationSelector =
      std::string("禰") + "\xF3\xA0\x84\x80";
  EXPECT_TRUE(UTF8Util::ContainsVariationSelector(
      bmpVariationSelector.c_str(), bmpVariationSelector.length()));
  EXPECT_TRUE(UTF8Util::ContainsVariationSelector(
      supplementaryVariationSelector.c_str(),
      supplementaryVariationSelector.length()));
  const std::string textWithoutVariationSelector = "東菄鶇䍶𠍀倲𩜍𢘐";
  EXPECT_FALSE(UTF8Util::ContainsVariationSelector(
      textWithoutVariationSelector.c_str(),
      textWithoutVariationSelector.length()));
}

TEST_F(UTF8UtilTest, GetByteMap) {
  std::vector<size_t> byteMap;
  UTF8Util::GetByteMap(text, 6, &byteMap);
  EXPECT_EQ(std::vector<size_t>({0, 3, 6, 9, 12, 16}), byteMap);
}

} // namespace opencc
