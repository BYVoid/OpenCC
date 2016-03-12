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

#pragma once

#include "Lexicon.hpp"
#include "TextDict.hpp"
#include "TestUtils.hpp"

namespace opencc {

class TextDictTestBase : public ::testing::Test {
protected:
  TextDictTestBase() : textDict(CreateTextDictForText()){};

  TextDictPtr CreateTextDictForText() const {
    LexiconPtr lexicon(new Lexicon);
    lexicon->Add(DictEntryFactory::New("BYVoid", "byv"));
    lexicon->Add(DictEntryFactory::New("zigzagzig", "zag"));
    lexicon->Add(DictEntryFactory::New(utf8("積羽沉舟"), utf8("羣輕折軸")));
    lexicon->Add(DictEntryFactory::New(utf8("清"), "Tsing"));
    lexicon->Add(DictEntryFactory::New(utf8("清華"), "Tsinghua"));
    lexicon->Add(DictEntryFactory::New(utf8("清華大學"), "TsinghuaUniversity"));
    lexicon->Sort();
    return TextDictPtr(new TextDict(lexicon));
  }

  DictPtr CreateDictForCharacters() const {
    LexiconPtr lexicon(new Lexicon);
    lexicon->Add(DictEntryFactory::New(utf8("后"),
                                       vector<string>{utf8("后"), utf8("後")}));
    lexicon->Add(DictEntryFactory::New(utf8("发"),
                                       vector<string>{utf8("發"), utf8("髮")}));
    lexicon->Add(DictEntryFactory::New(
        utf8("干"), vector<string>{utf8("幹"), utf8("乾"), utf8("干")}));
    lexicon->Add(DictEntryFactory::New(utf8("里"),
                                       vector<string>{utf8("裏"), utf8("里")}));
    lexicon->Sort();
    return TextDictPtr(new TextDict(lexicon));
  }

  DictPtr CreateDictForPhrases() const {
    LexiconPtr lexicon(new Lexicon);
    lexicon->Add(DictEntryFactory::New(utf8("太后"), utf8("太后")));
    lexicon->Add(DictEntryFactory::New(utf8("头发"), utf8("頭髮")));
    lexicon->Add(DictEntryFactory::New(utf8("干燥"), utf8("乾燥")));
    lexicon->Add(DictEntryFactory::New(utf8("鼠标"), utf8("鼠標")));
    lexicon->Sort();
    return TextDictPtr(new TextDict(lexicon));
  }

  DictPtr CreateDictForTaiwanVariants() const {
    LexiconPtr lexicon(new Lexicon);
    lexicon->Add(DictEntryFactory::New(utf8("裏"), utf8("裡")));
    TextDictPtr textDict(new TextDict(lexicon));
    return textDict;
  }

  DictPtr CreateTaiwanPhraseDict() const {
    LexiconPtr lexicon(new Lexicon);
    lexicon->Add(DictEntryFactory::New(utf8("鼠标"), utf8("滑鼠")));
    lexicon->Add(DictEntryFactory::New(utf8("服务器"), utf8("伺服器")));
    lexicon->Add(DictEntryFactory::New(utf8("克罗地亚"), utf8("克羅埃西亞")));
    lexicon->Sort();
    return TextDictPtr(new TextDict(lexicon));
  }

  void TestDict(const DictPtr dict) const {
    Optional<const DictEntry*> entry = dict->MatchPrefix("BYVoid");
    EXPECT_TRUE(!entry.IsNull());
    EXPECT_EQ(utf8("BYVoid"), entry.Get()->Key());
    EXPECT_EQ(utf8("byv"), entry.Get()->GetDefault());

    entry = dict->MatchPrefix("BYVoid123");
    EXPECT_TRUE(!entry.IsNull());
    EXPECT_EQ(utf8("BYVoid"), entry.Get()->Key());
    EXPECT_EQ(utf8("byv"), entry.Get()->GetDefault());

    entry = dict->MatchPrefix(utf8("積羽沉舟"));
    EXPECT_TRUE(!entry.IsNull());
    EXPECT_EQ(utf8("積羽沉舟"), entry.Get()->Key());
    EXPECT_EQ(utf8("羣輕折軸"), entry.Get()->GetDefault());

    entry = dict->MatchPrefix("Unknown");
    EXPECT_TRUE(entry.IsNull());

    const vector<const DictEntry*> matches =
        dict->MatchAllPrefixes(utf8("清華大學計算機系"));
    EXPECT_EQ(3, matches.size());
    EXPECT_EQ(utf8("清華大學"), matches.at(0)->Key());
    EXPECT_EQ(utf8("TsinghuaUniversity"), matches.at(0)->GetDefault());
    EXPECT_EQ(utf8("清華"), matches.at(1)->Key());
    EXPECT_EQ(utf8("Tsinghua"), matches.at(1)->GetDefault());
    EXPECT_EQ(utf8("清"), matches.at(2)->Key());
    EXPECT_EQ(utf8("Tsing"), matches.at(2)->GetDefault());
  }

  const TextDictPtr textDict;
};

} // namespace opencc
