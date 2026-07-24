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
#include "Utf8SkipScan.hpp"

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

TEST_F(PrefixMatchTest, MatchPrefixViewFastPathReturnsViews) {
  // Single-dict fast-path: key view should point into caller's input buffer,
  // value view should point into the dict's stable storage (no copy).
  PrefixMatch pm(marisaDict);

  const std::string query = utf8("清華大學校園");
  PrefixMatchView v = pm.MatchPrefixView(query.c_str(), query.length());

  EXPECT_TRUE(v.matched);
  EXPECT_EQ(utf8("清華大學"), std::string(v.key));
  EXPECT_EQ(utf8("TsinghuaUniversity"), std::string(v.value));
  EXPECT_EQ(v.key.size(), v.keyLength);
  // key view must alias the caller's buffer exactly.
  EXPECT_EQ(query.data(), v.key.data());
}

TEST_F(PrefixMatchTest, MatchPrefixViewTablePathReturnsViews) {
  // Table-path (multi-dict): value view points into stable PrefixMatch storage.
  std::list<DictPtr> dictsMulti = {marisaDict, textDict};
  DictPtr dictGroupMulti(new DictGroup(dictsMulti));
  PrefixMatch pm(dictGroupMulti);

  const std::string query = utf8("清華大學校園");
  PrefixMatchView v = pm.MatchPrefixView(query.c_str(), query.length());

  EXPECT_TRUE(v.matched);
  EXPECT_EQ(utf8("清華大學"), std::string(v.key));
  EXPECT_EQ(utf8("TsinghuaUniversity"), std::string(v.value));
  EXPECT_EQ(v.key.size(), v.keyLength);
}

TEST_F(PrefixMatchTest, ShortCircuitGroupPrefersEarlierShorterMatch) {
  LexiconPtr firstLexicon(new Lexicon);
  firstLexicon->Add(DictEntryFactory::New(utf8("意大利"), utf8("義大利")));
  firstLexicon->Sort();
  DictPtr firstDict(new TextDict(firstLexicon));

  LexiconPtr secondLexicon(new Lexicon);
  secondLexicon->Add(DictEntryFactory::New(utf8("意大利面"), utf8("義大利麵")));
  secondLexicon->Sort();
  DictPtr secondDict(new TextDict(secondLexicon));

  DictPtr dictGroup(new DictGroup(
      std::list<DictPtr>{firstDict, secondDict},
      DictGroupMatchPolicy::ShortCircuit));
  PrefixMatch pm(dictGroup);

  const std::string query = utf8("意大利面");
  PrefixMatch::Match m = pm.MatchPrefix(query.c_str(), query.length());

  EXPECT_TRUE(m.matched);
  EXPECT_EQ(utf8("意大利"), *m.key);
  EXPECT_EQ(utf8("義大利"), *m.value);
}

TEST_F(PrefixMatchTest, UnionGroupPrefersLaterLongerMatch) {
  LexiconPtr firstLexicon(new Lexicon);
  firstLexicon->Add(DictEntryFactory::New(utf8("意大利"), utf8("義大利")));
  firstLexicon->Sort();
  DictPtr firstDict(new TextDict(firstLexicon));

  LexiconPtr secondLexicon(new Lexicon);
  secondLexicon->Add(DictEntryFactory::New(utf8("意大利面"), utf8("義大利麵")));
  secondLexicon->Sort();
  DictPtr secondDict(new TextDict(secondLexicon));

  DictPtr dictGroup(
      new UnionDictGroup(std::list<DictPtr>{firstDict, secondDict}));
  PrefixMatch pm(dictGroup);

  const std::string query = utf8("意大利面");
  PrefixMatch::Match m = pm.MatchPrefix(query.c_str(), query.length());

  EXPECT_TRUE(m.matched);
  EXPECT_EQ(utf8("意大利面"), *m.key);
  EXPECT_EQ(utf8("義大利麵"), *m.value);
}

TEST_F(PrefixMatchTest, UnionGroupPreservesNestedShortCircuitBoundary) {
  LexiconPtr firstLexicon(new Lexicon);
  firstLexicon->Add(DictEntryFactory::New(utf8("意"), "nested-first"));
  firstLexicon->Sort();
  DictPtr firstDict(new TextDict(firstLexicon));

  LexiconPtr hiddenLexicon(new Lexicon);
  hiddenLexicon->Add(DictEntryFactory::New(utf8("意大利面"), "hidden"));
  hiddenLexicon->Sort();
  DictPtr hiddenDict(new TextDict(hiddenLexicon));

  LexiconPtr rootLexicon(new Lexicon);
  rootLexicon->Add(DictEntryFactory::New(utf8("意大利"), "root-second"));
  rootLexicon->Sort();
  DictPtr rootDict(new TextDict(rootLexicon));

  DictPtr nestedGroup(
      new DictGroup(std::list<DictPtr>{firstDict, hiddenDict}));
  DictPtr rootGroup(
      new UnionDictGroup(std::list<DictPtr>{nestedGroup, rootDict}));
  PrefixMatch pm(rootGroup);

  const std::string query = utf8("意大利面");
  PrefixMatch::Match m = pm.MatchPrefix(query.c_str(), query.length());

  EXPECT_TRUE(m.matched);
  EXPECT_EQ(utf8("意大利"), *m.key);
  EXPECT_EQ("root-second", *m.value);
}

TEST(Utf8SkipScanTest, AsciiRunLength) {
  const std::string cjk = utf8("一");
  // Cover all vector-width boundaries and unaligned starting offsets.
  for (size_t asciiLength = 0; asciiLength < 70; asciiLength++) {
    const std::string text = std::string(asciiLength, 'a') + cjk;
    for (size_t offset = 0; offset <= asciiLength; offset++) {
      EXPECT_EQ(asciiLength - offset,
                internal::AsciiRunLength(text.data() + offset,
                                         text.size() - offset));
    }
  }
  const std::string pureAscii(100, 'x');
  EXPECT_EQ(pureAscii.size(),
            internal::AsciiRunLength(pureAscii.data(), pureAscii.size()));
  EXPECT_EQ(0u, internal::AsciiRunLength(cjk.data(), cjk.size()));
  EXPECT_EQ(0u, internal::AsciiRunLength("", 0));
}

TEST_F(PrefixMatchTest, SkipUnmatchableStopsAtAsciiCandidates) {
  // textDict contains ASCII keys ("BYVoid", "zigzagzig"), so the scalar
  // ASCII path must stop at candidate bytes 'B' and 'z'.
  PrefixMatch pm(marisaDict);
  const std::string query = "abc BYVoid";
  EXPECT_EQ(4u, pm.SkipUnmatchable(query.data(), query.size()));
  EXPECT_EQ(0u, pm.SkipUnmatchable(query.data() + 4, query.size() - 4));
  const std::string zQuery = "zoo";
  EXPECT_EQ(0u, pm.SkipUnmatchable(zQuery.data(), zQuery.size()));
}

TEST_F(PrefixMatchTest, SkipNeverCrossesAnyKeyFirstChar) {
  // Invariant: for every dictionary key, the skip scan must not consume the
  // position where that key starts, on both the fast path and the table
  // path. A false skip here would silently corrupt conversion output.
  PrefixMatch pmFast(marisaDict);
  std::list<DictPtr> dictsMulti = {marisaDict, textDict};
  DictPtr dictGroupMulti(new DictGroup(dictsMulti));
  PrefixMatch pmTable(dictGroupMulti);
  const LexiconPtr lexicon = textDict->GetLexicon();
  for (const std::unique_ptr<DictEntry>& entry : *lexicon) {
    const std::string& key = entry->Key();
    EXPECT_EQ(0u, pmFast.SkipUnmatchable(key.data(), key.length())) << key;
    EXPECT_EQ(0u, pmTable.SkipUnmatchable(key.data(), key.length())) << key;
  }
}

class PrefixMatchSkipTest : public ::testing::Test {
protected:
  PrefixMatchSkipTest() {
    LexiconPtr lexicon(new Lexicon);
    lexicon->Add(DictEntryFactory::New(utf8("太后"), utf8("太后")));
    lexicon->Add(DictEntryFactory::New(utf8("里"), utf8("裏")));
    lexicon->Sort();
    dict = TextDictPtr(new TextDict(lexicon));
  }

  TextDictPtr dict;
};

TEST_F(PrefixMatchSkipTest, SkipsVectorizedAsciiRuns) {
  // No ASCII key exists, so long ASCII runs take the vectorized scan.
  PrefixMatch pm(dict);
  const std::string ascii(100, 'x');
  EXPECT_EQ(ascii.size(), pm.SkipUnmatchable(ascii.data(), ascii.size()));
  const std::string mixed = ascii + utf8("太后");
  EXPECT_EQ(ascii.size(), pm.SkipUnmatchable(mixed.data(), mixed.size()));
}

TEST_F(PrefixMatchSkipTest, SkipsNonCandidateCjkCharacters) {
  // 、。「」 and kana share the lead byte 0xE3 which starts no key; they are
  // skipped without a lookup. 太 (0xE5) is a candidate and stops the scan.
  PrefixMatch pm(dict);
  const std::string punct = utf8("、。「」あい");
  EXPECT_EQ(punct.size(), pm.SkipUnmatchable(punct.data(), punct.size()));
  const std::string mixed = punct + utf8("太后");
  EXPECT_EQ(punct.size(), pm.SkipUnmatchable(mixed.data(), mixed.size()));
  const std::string candidate = utf8("太x");
  EXPECT_EQ(0u, pm.SkipUnmatchable(candidate.data(), candidate.size()));
}

TEST_F(PrefixMatchSkipTest, FourByteCharactersUseLeadByteFiltering) {
  // No 4-byte key exists in dict, so lead byte 0xF0 is not a candidate and
  // 4-byte characters are skipped whole.
  PrefixMatch pm(dict);
  const std::string emoji = utf8("🎉🚀x太后");
  EXPECT_EQ(9u, pm.SkipUnmatchable(emoji.data(), emoji.size()));

  // With a 4-byte key, its lead byte becomes a candidate; any 4-byte
  // character sharing that lead byte conservatively stops the scan.
  LexiconPtr lexicon(new Lexicon);
  lexicon->Add(DictEntryFactory::New(utf8("𠀀"), "X"));
  lexicon->Sort();
  DictPtr extDict(new TextDict(lexicon));
  PrefixMatch pmExt(extDict);
  const std::string extB = utf8("𠀀");
  const std::string otherFourByte = utf8("🎉");
  EXPECT_EQ(0u, pmExt.SkipUnmatchable(extB.data(), extB.size()));
  EXPECT_EQ(0u, pmExt.SkipUnmatchable(otherFourByte.data(),
                                      otherFourByte.size()));
}

TEST_F(PrefixMatchSkipTest, CharLevelSkipsSameLeadByteNonCandidates) {
  // Table path uses character-granularity filtering: 天/地 share the lead
  // byte 0xE5 with the key 太后 but are not candidate first characters, so
  // they are skipped; 太 itself stops the scan.
  PrefixMatch pm(dict);
  const std::string query = utf8("天地太后");
  EXPECT_EQ(6u, pm.SkipUnmatchable(query.data(), query.size()));
  // 黑 shares the lead byte 0xE9 with the key 里.
  const std::string query2 = utf8("黑里");
  EXPECT_EQ(3u, pm.SkipUnmatchable(query2.data(), query2.size()));
}

TEST_F(PrefixMatchSkipTest, CharLevelSkipsNonIdsPunctuationWithLead0xE2) {
  // Em dash and curly quotes share the lead byte 0xE2 with the IDS
  // operators but are not operators themselves; character-level filtering
  // skips them while U+2FF0..U+2FFF still stop the scan.
  PrefixMatch pm(dict);
  const std::string dashes = utf8("——“”…");
  EXPECT_EQ(dashes.size(), pm.SkipUnmatchable(dashes.data(), dashes.size()));
  const std::string ids = utf8("—⿰氵青");
  EXPECT_EQ(3u, pm.SkipUnmatchable(ids.data(), ids.size()));
}

TEST_F(PrefixMatchSkipTest, CharLevelSkipsTwoByteCharacters) {
  PrefixMatch pm(dict);
  const std::string query = utf8("Ünïcodé tail 太");
  EXPECT_EQ(query.size() - 3, pm.SkipUnmatchable(query.data(), query.size()));
}

TEST_F(PrefixMatchSkipTest, AllIdsOperatorsStopTheScan) {
  // Pins Finalize() to the arity table: every recognized IDS operator must
  // stop the scan even though none of them begins a dictionary key, so the
  // conversion loop's IDS grouping still sees them.
  PrefixMatch pm(dict);
  for (uint32_t cp = UTF8Util::kFirstIdeographicDescriptionOperator;
       cp <= UTF8Util::kLastIdeographicDescriptionOperator; cp++) {
    if (UTF8Util::IdeographicDescriptionOperatorArity(cp) == 0) {
      continue;
    }
    const char encoded[3] = {
        static_cast<char>(0xE0 | (cp >> 12)),
        static_cast<char>(0x80 | ((cp >> 6) & 0x3F)),
        static_cast<char>(0x80 | (cp & 0x3F)),
    };
    EXPECT_EQ(0u, pm.SkipUnmatchable(encoded, 3)) << "U+" << std::hex << cp;
  }
}

TEST_F(PrefixMatchSkipTest, StopsAtIdeographicDescriptionOperator) {
  // No key starts with 0xE2, but IDS operators (U+2FF0..U+2FFF) must still
  // reach the conversion loop so IDS grouping is preserved.
  PrefixMatch pm(dict);
  const std::string query = "abc" + utf8("⿰氵青");
  EXPECT_EQ(3u, pm.SkipUnmatchable(query.data(), query.size()));
  EXPECT_EQ(0u, pm.SkipUnmatchable(query.data() + 3, query.size() - 3));
}

TEST_F(PrefixMatchSkipTest, StopsAtInvalidOrTruncatedSequences) {
  PrefixMatch pm(dict);
  const std::string invalid = std::string("ab") + '\xFF';
  EXPECT_EQ(2u, pm.SkipUnmatchable(invalid.data(), invalid.size()));
  // Truncated 3-byte sequence: lead byte expects more bytes than remain.
  const std::string truncated = std::string("ab") + '\xE3';
  EXPECT_EQ(2u, pm.SkipUnmatchable(truncated.data(), truncated.size()));
  EXPECT_EQ(0u, pm.SkipUnmatchable(invalid.data() + 2, 1));
}

TEST_F(PrefixMatchTest, SkipUnmatchableTablePath) {
  // Multi-dict group exercises the table path; skip bytes are the union of
  // all leaf dictionaries' key start bytes.
  std::list<DictPtr> dictsMulti = {marisaDict, textDict};
  DictPtr dictGroupMulti(new DictGroup(dictsMulti));
  PrefixMatch pm(dictGroupMulti);
  const std::string query = utf8("abc 清華");
  EXPECT_EQ(4u, pm.SkipUnmatchable(query.data(), query.size()));
  EXPECT_EQ(0u, pm.SkipUnmatchable(query.data() + 4, query.size() - 4));
}

} // namespace opencc
