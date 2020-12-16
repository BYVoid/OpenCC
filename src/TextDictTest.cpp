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

#include "TestUtilsUTF8.hpp"
#include "TextDictTestBase.hpp"

namespace opencc {

class TextDictTest : public TextDictTestBase {
protected:
  TextDictTest() : fileName("dict.txt"){};

  const std::string fileName;
};

TEST_F(TextDictTest, DictTest) { TestDict(textDict); }

TEST_F(TextDictTest, Serialization) {
  textDict->opencc::SerializableDict::SerializeToFile(fileName);
}

TEST_F(TextDictTest, Deserialization) {
  const TextDictPtr& deserialized =
      SerializableDict::NewFromFile<TextDict>(fileName);
  TestDict(deserialized);
}

TEST_F(TextDictTest, ExactMatch) {
  auto there = textDict->Match("積羽沉舟", 12);
  EXPECT_FALSE(there.IsNull());
  auto dictEntry = there.Get();
  EXPECT_EQ(1, dictEntry->NumValues());
  EXPECT_EQ(utf8("羣輕折軸"), dictEntry->GetDefault());

  auto nowhere = textDict->Match("積羽沉舟衆口鑠金", 24);
  EXPECT_TRUE(nowhere.IsNull());
}

} // namespace opencc
