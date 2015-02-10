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
  const string input;
  const string expected;
};

TEST_F(ConversionTest, ConvertString) {
  const string converted = conversion->Convert(input);
  EXPECT_EQ(expected, converted);
}

TEST_F(ConversionTest, ConvertCString) {
  const string converted = conversion->Convert(input.c_str());
  EXPECT_EQ(expected, converted);
}

} // namespace opencc
