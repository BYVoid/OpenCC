/*
 * Open Chinese Convert
 *
 * Copyright 2010-2026 Carbo Kuo and contributors
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

#include "ConversionCandidates.hpp"

#include "ConfigBasedConverter.hpp"
#include "Conversion.hpp"
#include "ConversionChain.hpp"
#include "Converter.hpp"
#include "Lexicon.hpp"
#include "MaxMatchSegmentation.hpp"
#include "PipelineConverter.hpp"
#include "SingleStageConverter.hpp"
#include "TestUtils.hpp"
#include "TestUtilsUTF8.hpp"
#include "TextDict.hpp"

namespace opencc {

class ConversionCandidatesTest : public ::testing::Test {
protected:
  static DictPtr MakeDict(
      const std::vector<std::pair<std::string, std::vector<std::string>>>&
          entries) {
    LexiconPtr lexicon(new Lexicon);
    for (const auto& entry : entries) {
      lexicon->Add(DictEntryFactory::New(entry.first, entry.second));
    }
    lexicon->Sort();
    return DictPtr(new TextDict(lexicon));
  }

  static ConverterPtr MakeConverter(const std::list<DictPtr>& dicts) {
    std::list<ConversionPtr> conversions;
    for (const DictPtr& dict : dicts) {
      conversions.push_back(ConversionPtr(new Conversion(dict)));
    }
    ConversionChainPtr chain(new ConversionChain(conversions));
    // Segmentation is irrelevant to word-level candidate lookup; any dict works.
    SegmentationPtr segmentation(
        new MaxMatchSegmentation(dicts.empty() ? MakeDict({}) : dicts.front()));
    return ConverterPtr(new SingleStageConverter(segmentation, chain));
  }
};

// A single dictionary with multiple values returns every value.
TEST_F(ConversionCandidatesTest, SingleDictMultipleValues) {
  const ConverterPtr converter =
      MakeConverter({MakeDict({{utf8("里"), {utf8("里"), utf8("裏")}}})});
  const std::vector<std::string> candidates =
      GetAllConversions(*converter, utf8("里"));
  EXPECT_EQ((std::vector<std::string>{utf8("里"), utf8("裏")}), candidates);
}

// Candidates from an earlier dictionary flow independently through a later one:
// s2t expands 里 -> {里, 裏}; t2tw then passes 里 through and maps 裏 -> 裡.
TEST_F(ConversionCandidatesTest, ChainedExpansionCarriesBranches) {
  const ConverterPtr converter = MakeConverter({
      MakeDict({{utf8("里"), {utf8("里"), utf8("裏")}}}),
      MakeDict({{utf8("裏"), {utf8("裡")}}}),
  });
  const std::vector<std::string> candidates =
      GetAllConversions(*converter, utf8("里"));
  EXPECT_EQ((std::vector<std::string>{utf8("里"), utf8("裡")}), candidates);
}

// Duplicate branches collapse: both 麵 and 面 map to 面 downstream.
TEST_F(ConversionCandidatesTest, DuplicateCandidatesAreDeduped) {
  const ConverterPtr converter = MakeConverter({
      MakeDict({{utf8("面"), {utf8("麵"), utf8("面")}}}),
      MakeDict({{utf8("麵"), {utf8("面")}}}),
  });
  const std::vector<std::string> candidates =
      GetAllConversions(*converter, utf8("面"));
  EXPECT_EQ((std::vector<std::string>{utf8("面")}), candidates);
}

// A word that no dictionary in the chain contains yields an empty result.
TEST_F(ConversionCandidatesTest, WordNotInAnyDictReturnsEmpty) {
  const ConverterPtr converter =
      MakeConverter({MakeDict({{utf8("里"), {utf8("裏")}}})});
  EXPECT_TRUE(GetAllConversions(*converter, utf8("国")).empty());
}

// Only a prefix (not the whole word) matches: no exact match means "not found",
// matching librime's ConvertWord contract.
TEST_F(ConversionCandidatesTest, PrefixOnlyMatchReturnsEmpty) {
  const ConverterPtr converter =
      MakeConverter({MakeDict({{utf8("里"), {utf8("裏")}}})});
  EXPECT_TRUE(GetAllConversions(*converter, utf8("里面")).empty());
}

// An exact match on a longer word still carries an unconvertible tail through a
// later dictionary via longest-prefix conversion.
TEST_F(ConversionCandidatesTest, PartialConversionInLaterDict) {
  const ConverterPtr converter = MakeConverter({
      MakeDict({{utf8("里面"), {utf8("裏面")}}}),
      MakeDict({{utf8("裏"), {utf8("裡")}}}),
  });
  const std::vector<std::string> candidates =
      GetAllConversions(*converter, utf8("里面"));
  // Stage 1: 里面 -> 裏面. Stage 2: 裏 -> 裡 (prefix), 面 copied -> 裡面.
  EXPECT_EQ((std::vector<std::string>{utf8("裡面")}), candidates);
}

// A converter without a single chain (PipelineConverter) yields no candidates.
TEST_F(ConversionCandidatesTest, PipelineConverterYieldsEmpty) {
  const ConverterPtr stage =
      MakeConverter({MakeDict({{utf8("里"), {utf8("裏")}}})});
  ConverterPtr pipeline(new PipelineConverter({stage}));
  EXPECT_EQ(nullptr, pipeline->GetConversionChain());
  EXPECT_TRUE(GetAllConversions(*pipeline, utf8("里")).empty());
}

// A key-only entry (no values) converts the word to itself, matching
// Convert(), instead of silently dropping the branch.
TEST_F(ConversionCandidatesTest, KeyOnlyEntryPassesWordThrough) {
  const ConverterPtr converter =
      MakeConverter({MakeDict({{utf8("通道"), {}}})});
  const std::vector<std::string> candidates =
      GetAllConversions(*converter, utf8("通道"));
  EXPECT_EQ((std::vector<std::string>{utf8("通道")}), candidates);
}

// Input ending in a truncated UTF-8 sequence must not read out of bounds; the
// longest-prefix fallback delegates to Conversion::Convert, which clamps.
TEST_F(ConversionCandidatesTest, TruncatedUtf8InputIsSafe) {
  const ConverterPtr converter =
      MakeConverter({MakeDict({{utf8("里"), {utf8("裏")}}})});
  // First two bytes of 里 (0xE9 0x87 0x8C): an incomplete 3-byte sequence.
  EXPECT_TRUE(GetAllConversions(*converter, "\xE9\x87").empty());
  // Truncated tail after a convertible prefix: no exact match anywhere.
  EXPECT_TRUE(GetAllConversions(*converter, utf8("里") + "\xE9\x87").empty());
}

// An ideographic description sequence flowing through a later dictionary's
// longest-prefix fallback stays atomic, matching Converter::Convert.
TEST_F(ConversionCandidatesTest, IdsKeptAtomicInLaterDict) {
  const ConverterPtr converter = MakeConverter({
      MakeDict({{utf8("里字"), {utf8("⿱艹里")}}}),
      MakeDict({{utf8("里"), {utf8("裏")}}}),
  });
  const std::vector<std::string> candidates =
      GetAllConversions(*converter, utf8("里字"));
  // Stage 2 must not convert the 里 inside the IDS ⿱艹里.
  EXPECT_EQ((std::vector<std::string>{utf8("⿱艹里")}), candidates);
}

// A config-based converter's normalization pre-pass is applied before the main
// chain, mirroring Convert(): a CJK Compatibility Ideograph key is normalized
// first and then expanded by the main chain.
TEST_F(ConversionCandidatesTest, NormalizationPrePassApplied) {
  // \xEF\xA7\xA9 is U+F9E9, the compatibility ideograph for 里.
  const ConverterPtr norm =
      MakeConverter({MakeDict({{"\xEF\xA7\xA9", {utf8("里")}}})});
  const ConverterPtr main =
      MakeConverter({MakeDict({{utf8("里"), {utf8("里"), utf8("裏")}}})});
  const ConverterPtr converter(new ConfigBasedConverter(norm, main));
  const std::vector<std::string> candidates =
      GetAllConversions(*converter, "\xEF\xA7\xA9");
  EXPECT_EQ((std::vector<std::string>{utf8("里"), utf8("裏")}), candidates);
}

} // namespace opencc
