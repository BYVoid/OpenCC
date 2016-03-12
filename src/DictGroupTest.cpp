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

#include "DictGroupTestBase.hpp"

namespace opencc {

class DictGroupTest : public DictGroupTestBase {
protected:
  DictGroupTest() {}
};

TEST_F(DictGroupTest, SimpleGroupTest) {
  const DictGroupPtr& dictGroup = CreateDictGroupForConversion();
  const auto& entry = dictGroup->Dict::MatchPrefix(utf8("Unknown"));
  EXPECT_TRUE(entry.IsNull());

  const auto& matches = dictGroup->Dict::MatchAllPrefixes(utf8("干燥"));
  EXPECT_EQ(2, matches.size());
  EXPECT_EQ(utf8("乾燥"), matches.at(0)->GetDefault());
  EXPECT_EQ(utf8("幹"), matches.at(1)->GetDefault());
}

TEST_F(DictGroupTest, TaiwanPhraseGroupTest) {
  const DictGroupPtr dictGroup(new DictGroup(
      list<DictPtr>{CreateDictForPhrases(), CreateTaiwanPhraseDict()}));
  {
    const auto& entry = dictGroup->Dict::MatchPrefix(utf8("鼠标"));
    EXPECT_EQ(utf8("鼠標"), entry.Get()->GetDefault());
  }
  {
    const auto& entry = dictGroup->Dict::MatchPrefix(utf8("克罗地亚"));
    EXPECT_EQ(utf8("克羅埃西亞"), entry.Get()->GetDefault());
  }
  {
    const auto& matches = dictGroup->Dict::MatchAllPrefixes(utf8("鼠标"));
    EXPECT_EQ(1, matches.size());
    EXPECT_EQ(utf8("鼠標"), matches[0]->GetDefault());
  }
}

} // namespace opencc
