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

#include "Conversion.hpp"
#include "DictGroupTestBase.hpp"

namespace opencc {

class ConversionTest : public DictGroupTestBase {
protected:
  ConversionTest()
      : input(utf8("太后的头发干燥")), expected(utf8("太后的頭髮乾燥")) {}

  virtual void SetUp() {
    dict = CreateDictGroupForConversion();
    conversion = ConversionPtr(new Conversion(dict));
  }

  DictPtr dict;
  ConversionPtr conversion;
  const std::string input;
  const std::string expected;
};

TEST_F(ConversionTest, ConvertString) {
  const std::string converted = conversion->Convert(input);
  EXPECT_EQ(expected, converted);
}

TEST_F(ConversionTest, ConvertCString) {
  const std::string converted = conversion->Convert(input.c_str());
  EXPECT_EQ(expected, converted);
}

TEST_F(ConversionTest, TruncatedUtf8Sequence) {
  // This test specifically triggers the information disclosure vulnerability
  // in the old code. The bug occurs when a string ends with an incomplete
  // UTF-8 sequence.
  //
  // Background: UTF8Util::NextCharLength() examines only the first byte to
  // determine the expected character length (1-6 bytes), but doesn't verify
  // that enough bytes actually remain before the null terminator.
  //
  // Trigger condition: When the expected UTF-8 character length exceeds
  // the actual remaining bytes before null, the old code would:
  // 1. Call FromSubstr with a length crossing the null terminator
  // 2. Advance pstr beyond the null terminator
  // 3. Continue reading heap memory on next iteration
  // 4. Output leaked heap data to conversion result (INFORMATION DISCLOSURE)

  // Construct a string ending with a truncated 3-byte UTF-8 sequence:
  // - Normal text: "干" (valid 3-byte UTF-8: 0xE5 0xB9 0xB2)
  // - Followed by: 0xE5 0xB9 (incomplete 3-byte sequence - missing last byte)
  std::string malformed;
  malformed += utf8("干");   // Valid character
  malformed += '\xE5';       // Start of 3-byte UTF-8 (NextCharLength returns 3)
  malformed += '\xB9';       // Second byte
  // Missing third byte - only 2 bytes remain but NextCharLength expects 3
  // Old code would jump over null, read heap memory, and leak it in output

  // The fixed code should handle this gracefully without information disclosure
  EXPECT_NO_THROW({
    const std::string converted = conversion->Convert(malformed);
    // Should convert "干" to "幹" (first candidate in dict) and preserve incomplete sequence
    std::string expected;
    expected += utf8("幹");  // Converted from "干" (dict has ["幹", "乾", "干"])
    expected += '\xE5';      // Incomplete sequence preserved as-is
    expected += '\xB9';
    EXPECT_EQ(expected, converted);
    // Should NOT contain garbage heap data beyond the input
    // (ASan would catch any out-of-bounds reads during conversion)
  });
}

} // namespace opencc
