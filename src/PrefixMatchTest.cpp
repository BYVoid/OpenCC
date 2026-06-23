/*
 * Open Chinese Convert
 *
 * Copyright 2020-2026 Carbo Kuo and contributors
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

#include <algorithm>
#include <cstring>
#include <list>
#include <string>
#include <vector>

#include "PrefixMatch.hpp"
#include "DictGroup.hpp"
#include "MarisaDict.hpp"
#include "TestUtilsUTF8.hpp"
#include "TextDictTestBase.hpp"

namespace opencc {

class PrefixMatchTest : public TextDictTestBase {
protected:
  PrefixMatchTest()
      : marisaDict(MarisaDict::NewFromDict(*textDict)), fileName("dict.ocd2") {
    marisaDict->opencc::SerializableDict::SerializeToFile(fileName);
  }

  virtual ~PrefixMatchTest() {
    std::remove(fileName.c_str());
  }

  const MarisaDictPtr marisaDict;
  const std::string fileName;
};

TEST_F(PrefixMatchTest, SingleDictFastPathMatchesTablePath) {
  // 1. Create a single-element DictGroup wrapper around marisaDict
  std::list<DictPtr> dicts = { marisaDict };
  DictPtr dictGroup(new DictGroup(dicts));

  // 2. Create two PrefixMatch instances:
  // - pmFast uses the single dict fast-path (unwrapped group -> marisaDict)
  PrefixMatch pmFast(dictGroup);

  // - pmTable uses the standard table path because we pass a group of two dicts
  std::list<DictPtr> dictsMulti = { marisaDict, textDict };
  DictPtr dictGroupMulti(new DictGroup(dictsMulti));
  PrefixMatch pmTable(dictGroupMulti);

  // Compare queries
  const std::vector<std::string> testQueries = {
    "清華", "清華大", "清華大學", "清", "積羽沉舟", "nowhere"
  };

  for (const std::string& query : testQueries) {
    PrefixMatch::Match mFast = pmFast.MatchPrefix(query.c_str(), query.length());
    PrefixMatch::Match mTable = pmTable.MatchPrefix(query.c_str(), query.length());

    EXPECT_EQ(mFast.matched, mTable.matched);
    if (mFast.matched) {
      EXPECT_EQ(mFast.keyLength, mTable.keyLength);
      EXPECT_EQ(*mFast.key, *mTable.key);
      EXPECT_EQ(*mFast.value, *mTable.value);
    }
  }
}

TEST_F(PrefixMatchTest, MarisaLazyLoadFastPathDoesNotReconstruct) {
  // 1. Load MarisaDict from the serialized ocd2 file (lazy load)
  MarisaDictPtr lazyDict = SerializableDict::NewFromFile<MarisaDict>(fileName);
  
  // Verify it is not reconstructed initially
  EXPECT_FALSE(lazyDict->IsLexiconReconstructed());

  // 2. Create PrefixMatch single-dict fast-path on it
  PrefixMatch pm(lazyDict);

  // 3. Query a prefix that should hit
  PrefixMatch::Match m = pm.MatchPrefix("清華", 6);
  EXPECT_TRUE(m.matched);
  EXPECT_EQ(6, m.keyLength); // "清華" length is 6 bytes in UTF-8
  EXPECT_EQ("清華", *m.key);
  EXPECT_EQ(utf8("Tsinghua"), *m.value);

  // 4. Verify that the lexicon was NOT reconstructed during the query!
  EXPECT_FALSE(lazyDict->IsLexiconReconstructed());
}

TEST_F(PrefixMatchTest, FastPathSelectsLongestMatch) {
  // Dictionary has "清"→Tsing, "清華"→Tsinghua, "清華大學"→TsinghuaUniversity.
  // A query with a longer input must match the longest key, not the first
  // prefix returned by common_prefix_search().
  PrefixMatch pm(marisaDict);

  const std::string query = utf8("清華大學校園");
  PrefixMatch::Match m = pm.MatchPrefix(query.c_str(), query.length());

  EXPECT_TRUE(m.matched);
  EXPECT_EQ(utf8("清華大學"), *m.key);
  EXPECT_EQ(utf8("TsinghuaUniversity"), *m.value);
  EXPECT_EQ(m.key->length(), m.keyLength);
}

} // namespace opencc
