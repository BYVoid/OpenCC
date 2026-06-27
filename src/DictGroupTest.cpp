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

#include "DictGroupTestBase.hpp"

namespace opencc {

class DictGroupTest : public DictGroupTestBase {
protected:
  DictGroupTest() {}
};

TEST_F(DictGroupTest, KeyMaxLength) {
  const DictGroupPtr& dictGroup = CreateDictGroupForConversion();
  EXPECT_EQ(6, dictGroup->KeyMaxLength());
  EXPECT_EQ(6, dictGroup->GetDicts().front()->KeyMaxLength());
  EXPECT_EQ(3, dictGroup->GetDicts().back()->KeyMaxLength());
}

TEST_F(DictGroupTest, MatchPolicyDefaultsToShortCircuit) {
  const DictGroupPtr& dictGroup = CreateDictGroupForConversion();
  EXPECT_EQ(DictGroupMatchPolicy::ShortCircuit, dictGroup->GetMatchPolicy());
}

TEST_F(DictGroupTest, ExplicitMatchPolicy) {
  const DictPtr phrasesDict = CreateDictForPhrases();
  const DictPtr charactersDict = CreateDictForCharacters();
  const DictGroupPtr dictGroup(new DictGroup(
      std::list<DictPtr>{phrasesDict, charactersDict},
      DictGroupMatchPolicy::ShortCircuit));
  EXPECT_EQ(DictGroupMatchPolicy::ShortCircuit, dictGroup->GetMatchPolicy());
}

TEST_F(DictGroupTest, UnionMatchPolicy) {
  const DictPtr phrasesDict = CreateDictForPhrases();
  const DictPtr charactersDict = CreateDictForCharacters();
  const DictGroupPtr dictGroup(
      new UnionDictGroup(std::list<DictPtr>{phrasesDict, charactersDict}));
  EXPECT_EQ(DictGroupMatchPolicy::Union, dictGroup->GetMatchPolicy());
}

TEST_F(DictGroupTest, UnionMatchPrefixPrefersLaterLongerMatch) {
  LexiconPtr firstLexicon(new Lexicon);
  firstLexicon->Add(DictEntryFactory::New(utf8("意大利"), utf8("義大利")));
  firstLexicon->Sort();
  DictPtr firstDict(new TextDict(firstLexicon));

  LexiconPtr secondLexicon(new Lexicon);
  secondLexicon->Add(DictEntryFactory::New(utf8("意大利面"), utf8("義大利麵")));
  secondLexicon->Sort();
  DictPtr secondDict(new TextDict(secondLexicon));

  const DictGroupPtr dictGroup(
      new UnionDictGroup(std::list<DictPtr>{firstDict, secondDict}));
  const std::string query = utf8("意大利面");
  const auto& entry = dictGroup->MatchPrefix(query.c_str(), query.length());
  EXPECT_FALSE(entry.IsNull());
  EXPECT_EQ(utf8("意大利面"), entry.Get()->Key());
  EXPECT_EQ(utf8("義大利麵"), entry.Get()->GetDefault());
}

TEST_F(DictGroupTest, SimpleGroupTest) {
  const DictGroupPtr& dictGroup = CreateDictGroupForConversion();
  {
    const auto& entry = dictGroup->Dict::MatchPrefix(utf8("Unknown"));
    EXPECT_TRUE(entry.IsNull());
  }
  {
    const auto& entry = dictGroup->Dict::MatchPrefix(utf8("里面"));
    EXPECT_FALSE(entry.IsNull());
    EXPECT_EQ(3, entry.Get()->KeyLength());
    EXPECT_EQ(utf8("裏"), entry.Get()->GetDefault());
  }
  {
    const auto& matches = dictGroup->Dict::MatchAllPrefixes(utf8("干燥"));
    EXPECT_EQ(2, matches.size());
    EXPECT_EQ(utf8("乾燥"), matches.at(0)->GetDefault());
    EXPECT_EQ(utf8("幹"), matches.at(1)->GetDefault());
  }
}

TEST_F(DictGroupTest, TaiwanPhraseGroupTest) {
  const DictGroupPtr dictGroup(new DictGroup(
      std::list<DictPtr>{CreateDictForPhrases(), CreateTaiwanPhraseDict()}));
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
