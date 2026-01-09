/*
 * Open Chinese Convert
 *
 * Copyright 2015 Carbo Kuo <byvoid@byvoid.com>
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

#include "MaxMatchSegmentation.hpp"
#include "DictGroupTestBase.hpp"

namespace opencc {

class MaxMatchSegmentationTest : public DictGroupTestBase {
protected:
  MaxMatchSegmentationTest() {}

  virtual void SetUp() {
    dict = CreateDictGroupForConversion();
    segmenter = SegmentationPtr(new MaxMatchSegmentation(dict));
  }

  DictPtr dict;
  SegmentationPtr segmenter;
};

TEST_F(MaxMatchSegmentationTest, Segment) {
  const auto& segments = segmenter->Segment(utf8("太后的头发干燥"));
  EXPECT_EQ(4, segments->Length());
  EXPECT_EQ(utf8("太后"), std::string(segments->At(0)));
  EXPECT_EQ(utf8("的"), std::string(segments->At(1)));
  EXPECT_EQ(utf8("头发"), std::string(segments->At(2)));
  EXPECT_EQ(utf8("干燥"), std::string(segments->At(3)));
}

TEST_F(MaxMatchSegmentationTest, EmptyString) {
  const auto& segments = segmenter->Segment("");
  EXPECT_EQ(0, segments->Length());
}

TEST_F(MaxMatchSegmentationTest, SingleCharacter) {
  const auto& segments = segmenter->Segment(utf8("一"));
  EXPECT_EQ(1, segments->Length());
  EXPECT_EQ(utf8("一"), std::string(segments->At(0)));
}

TEST_F(MaxMatchSegmentationTest, TruncatedUtf8Sequence) {
  // This test specifically triggers the buffer overflow bug in the old code.
  // The bug occurs when a string ends with an incomplete UTF-8 sequence.
  //
  // Background: UTF8Util::NextCharLength() examines only the first byte to
  // determine the expected character length (1-6 bytes), but doesn't verify
  // that enough bytes actually remain in the buffer.
  //
  // Trigger condition: When the expected UTF-8 character length exceeds
  // the actual remaining bytes, the old code's "length -= matchedLength"
  // causes integer underflow (size_t wraps around to a huge value), leading
  // to out-of-bounds reads in the next MatchPrefix() call.

  // Construct a string ending with a truncated 3-byte UTF-8 sequence:
  // - Normal text: "一" (valid 3-byte UTF-8: 0xE4 0xB8 0x80)
  // - Followed by: 0xE4 0xB8 (incomplete 3-byte sequence - missing last byte)
  std::string malformed;
  malformed += utf8("一");  // Valid character
  malformed += '\xE4';      // Start of 3-byte UTF-8 (NextCharLength returns 3)
  malformed += '\xB8';      // Second byte
  // Missing third byte - only 2 bytes remain but NextCharLength expects 3
  // Old code: length=2, matchedLength=3 → length = 2-3 = SIZE_MAX (underflow)

  // The fixed code should handle this gracefully without buffer overflow
  EXPECT_NO_THROW({
    const auto& segments = segmenter->Segment(malformed);
    // The valid character "一" plus the incomplete sequence form a single segment
    // (incomplete sequence doesn't match dictionary, gets accumulated with previous)
    EXPECT_EQ(1, segments->Length());
    // Output should preserve all input bytes (including incomplete sequence)
    // This is correct behavior - we don't discard data, we preserve it
    EXPECT_EQ(malformed, std::string(segments->At(0)));
  });
}

} // namespace opencc
