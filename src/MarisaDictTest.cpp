/*
 * Open Chinese Convert
 *
 * Copyright 2020-2021 Carbo Kuo <byvoid@byvoid.com>
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

#include "MarisaDict.hpp"
#include "TestUtilsUTF8.hpp"
#include "TextDictTestBase.hpp"

namespace opencc {

class MarisaDictTest : public TextDictTestBase {
protected:
  MarisaDictTest()
      : dict(MarisaDict::NewFromDict(*textDict)), fileName("dict.ocd2"){};

  const MarisaDictPtr dict;
  const std::string fileName;
};

TEST_F(MarisaDictTest, DictTest) { TestDict(dict); }

TEST_F(MarisaDictTest, Serialization) {
  dict->opencc::SerializableDict::SerializeToFile(fileName);
}

TEST_F(MarisaDictTest, Deserialization) {
  const MarisaDictPtr& deserialized =
      SerializableDict::NewFromFile<MarisaDict>(fileName);
  TestDict(deserialized);

  const LexiconPtr& lex1 = dict->GetLexicon();
  const LexiconPtr& lex2 = deserialized->GetLexicon();

  // Compare every entry
  EXPECT_EQ(lex1->Length(), lex2->Length());
  for (size_t i = 0; i < lex1->Length(); i++) {
    EXPECT_EQ(lex1->At(i)->Key(), lex2->At(i)->Key());
    EXPECT_EQ(lex1->At(i)->NumValues(), lex2->At(i)->NumValues());
  }
}

TEST_F(MarisaDictTest, ExactMatch) {
  auto there = dict->Match("積羽沉舟", 12);
  EXPECT_FALSE(there.IsNull());
  auto dictEntry = there.Get();
  EXPECT_EQ(1, dictEntry->NumValues());
  EXPECT_EQ(utf8("羣輕折軸"), dictEntry->GetDefault());

  auto nowhere = dict->Match("積羽沉舟衆口鑠金", 24);
  EXPECT_TRUE(nowhere.IsNull());
}

TEST_F(MarisaDictTest, MatchPrefix) {
  {
    auto there = dict->MatchPrefix("清華", 3);
    EXPECT_FALSE(there.IsNull());
    auto dictEntry = there.Get();
    EXPECT_EQ(utf8("Tsing"), dictEntry->GetDefault());
  }
  {
    auto there = dict->MatchPrefix("清華", 5);
    EXPECT_FALSE(there.IsNull());
    auto dictEntry = there.Get();
    EXPECT_EQ(utf8("Tsing"), dictEntry->GetDefault());
  }
  {
    auto there = dict->MatchPrefix("清華", 6);
    EXPECT_FALSE(there.IsNull());
    auto dictEntry = there.Get();
    EXPECT_EQ(utf8("Tsinghua"), dictEntry->GetDefault());
  }
  {
    auto there = dict->MatchPrefix("清華", 100);
    EXPECT_FALSE(there.IsNull());
    auto dictEntry = there.Get();
    EXPECT_EQ(utf8("Tsinghua"), dictEntry->GetDefault());
  }
}

} // namespace opencc
