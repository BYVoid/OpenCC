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

#include "ConversionAmbiguities.hpp"

#include "Conversion.hpp"
#include "ConversionChain.hpp"
#include "Converter.hpp"
#include "Lexicon.hpp"
#include "MaxMatchSegmentation.hpp"
#include "SingleStageConverter.hpp"
#include "TestUtils.hpp"
#include "TestUtilsUTF8.hpp"
#include "TextDict.hpp"

namespace opencc {

class ConversionAmbiguitiesTest : public ::testing::Test {
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
    SegmentationPtr segmentation(
        new MaxMatchSegmentation(dicts.empty() ? MakeDict({}) : dicts.front()));
    return ConverterPtr(new SingleStageConverter(segmentation, chain));
  }
};

TEST_F(ConversionAmbiguitiesTest, SingleStageMultiValue) {
  const ConverterPtr converter =
      MakeConverter({MakeDict({{"a", {"x"}}, {"b", {"y", "z"}}})});
  const AnnotatedConversion result =
      ConvertWithAmbiguities(*converter, "aba");
  EXPECT_EQ("xyx", result.output);
  EXPECT_EQ(converter->Convert("aba"), result.output);
  ASSERT_EQ(1, result.ambiguities.size());
  EXPECT_EQ(1, result.ambiguities[0].outputOffset);
  EXPECT_EQ(1, result.ambiguities[0].outputLength);
  ASSERT_EQ(1, result.sources.size());
  EXPECT_EQ("b", result.sources[result.ambiguities[0].sourceIndex]);
}

TEST_F(ConversionAmbiguitiesTest, SourcesAreDeduplicated) {
  const ConverterPtr converter =
      MakeConverter({MakeDict({{"b", {"y", "z"}}})});
  const AnnotatedConversion result =
      ConvertWithAmbiguities(*converter, "bcb");
  EXPECT_EQ("ycy", result.output);
  ASSERT_EQ(2, result.ambiguities.size());
  EXPECT_EQ(0, result.ambiguities[0].outputOffset);
  EXPECT_EQ(2, result.ambiguities[1].outputOffset);
  ASSERT_EQ(1, result.sources.size());
  EXPECT_EQ("b", result.sources[0]);
  EXPECT_EQ(0, result.ambiguities[0].sourceIndex);
  EXPECT_EQ(0, result.ambiguities[1].sourceIndex);
}

TEST_F(ConversionAmbiguitiesTest, AmbiguityInLaterStageMapsToInputSource) {
  // Stage 1 rewrites b -> y unambiguously; stage 2 has the one-to-many
  // entry on the intermediate form y.  The reported source must be the
  // original input slice "b", not the intermediate "y".
  const ConverterPtr converter =
      MakeConverter({MakeDict({{"b", {"y"}}}),
                     MakeDict({{"y", {"m", "n"}}})});
  const AnnotatedConversion result = ConvertWithAmbiguities(*converter, "ab");
  EXPECT_EQ("am", result.output);
  EXPECT_EQ(converter->Convert("ab"), result.output);
  ASSERT_EQ(1, result.ambiguities.size());
  EXPECT_EQ(1, result.ambiguities[0].outputOffset);
  EXPECT_EQ(1, result.ambiguities[0].outputLength);
  EXPECT_EQ("b", result.sources[result.ambiguities[0].sourceIndex]);
}

TEST_F(ConversionAmbiguitiesTest, StraddlingMatchCoarsensSpan) {
  // Stage 1 converts a -> x and b -> y (b is one-to-many); stage 2 matches
  // the phrase xy across the stage-1 boundary.  Conversion is applied per
  // segment, so straddling requires both stage-1 pieces to fall inside one
  // segment: use a segmentation dictionary that keeps "ab" whole.  The
  // ambiguous span must widen to cover the straddled range, source "ab".
  std::list<ConversionPtr> conversions{
      ConversionPtr(
          new Conversion(MakeDict({{"a", {"x"}}, {"b", {"y", "z"}}}))),
      ConversionPtr(new Conversion(MakeDict({{"xy", {"Q"}}})))};
  const ConverterPtr converter(new SingleStageConverter(
      SegmentationPtr(new MaxMatchSegmentation(MakeDict({{"ab", {"ab"}}}))),
      ConversionChainPtr(new ConversionChain(conversions))));
  const AnnotatedConversion result = ConvertWithAmbiguities(*converter, "ab");
  EXPECT_EQ("Q", result.output);
  EXPECT_EQ(converter->Convert("ab"), result.output);
  ASSERT_EQ(1, result.ambiguities.size());
  EXPECT_EQ(0, result.ambiguities[0].outputOffset);
  EXPECT_EQ(1, result.ambiguities[0].outputLength);
  EXPECT_EQ("ab", result.sources[result.ambiguities[0].sourceIndex]);
}

TEST_F(ConversionAmbiguitiesTest, UnmatchedTextHasNoAmbiguities) {
  const ConverterPtr converter = MakeConverter({MakeDict({{"b", {"y"}}})});
  const AnnotatedConversion result =
      ConvertWithAmbiguities(*converter, "hello");
  EXPECT_EQ("hello", result.output);
  EXPECT_TRUE(result.ambiguities.empty());
  EXPECT_TRUE(result.sources.empty());
}

TEST_F(ConversionAmbiguitiesTest, PhraseEntryShadowsAmbiguousCharacter) {
  // Mirrors the 干燥 vs 文丑 distinction: a phrase entry with a single value
  // wins over the ambiguous single character, so only the bare character
  // occurrence is reported.
  const ConverterPtr converter = MakeConverter(
      {MakeDict({{"b", {"y", "z"}}, {"ab", {"W"}}})});
  const AnnotatedConversion result = ConvertWithAmbiguities(*converter, "abcb");
  EXPECT_EQ("Wcy", result.output);
  EXPECT_EQ(converter->Convert("abcb"), result.output);
  ASSERT_EQ(1, result.ambiguities.size());
  EXPECT_EQ(2, result.ambiguities[0].outputOffset);
  EXPECT_EQ("b", result.sources[result.ambiguities[0].sourceIndex]);
}

TEST_F(ConversionAmbiguitiesTest, ChunkedStreamMatchesWholeInput) {
  const ConverterPtr converter =
      MakeConverter({MakeDict({{"a", {"x"}}, {"b", {"y", "z"}}, {"c", {"p", "q"}}})});
  std::string input;
  for (int i = 0; i < 100; i++) {
    input += "abcab";
  }
  const AnnotatedConversion whole = ConvertWithAmbiguities(*converter, input);

  AmbiguityStream stream(converter);
  std::string streamedOutput;
  std::vector<std::string> streamedSources;
  std::vector<AmbiguousSpan> streamedSpans;
  auto collect = [&](const AmbiguityStream::Chunk& chunk) {
    for (const std::string& source : chunk.newSources) {
      streamedSources.push_back(source);
    }
    for (const AmbiguousSpan& span : chunk.ambiguities) {
      streamedSpans.push_back(AmbiguousSpan{
          streamedOutput.size() + span.outputOffset, span.outputLength,
          span.sourceIndex});
    }
    streamedOutput += chunk.output;
  };
  // Feed in deliberately awkward 7-byte chunks.
  for (size_t pos = 0; pos < input.size(); pos += 7) {
    collect(stream.ConvertChunk(
        std::string_view(input).substr(pos, 7)));
  }
  collect(stream.Finish());

  EXPECT_EQ(whole.output, streamedOutput);
  EXPECT_EQ(whole.sources, streamedSources);
  ASSERT_EQ(whole.ambiguities.size(), streamedSpans.size());
  for (size_t i = 0; i < streamedSpans.size(); i++) {
    EXPECT_EQ(whole.ambiguities[i].outputOffset,
              streamedSpans[i].outputOffset);
    EXPECT_EQ(whole.ambiguities[i].outputLength,
              streamedSpans[i].outputLength);
    EXPECT_EQ(whole.sources[whole.ambiguities[i].sourceIndex],
              streamedSources[streamedSpans[i].sourceIndex]);
  }
}

TEST_F(ConversionAmbiguitiesTest, Utf8OffsetsAreBytes) {
  const ConverterPtr converter = MakeConverter(
      {MakeDict({{utf8("丑"), {utf8("醜"), utf8("丑")}}})});
  const std::string input = utf8("文丑");
  const AnnotatedConversion result = ConvertWithAmbiguities(*converter, input);
  EXPECT_EQ(utf8("文醜"), result.output);
  ASSERT_EQ(1, result.ambiguities.size());
  EXPECT_EQ(3, result.ambiguities[0].outputOffset);
  EXPECT_EQ(3, result.ambiguities[0].outputLength);
  EXPECT_EQ(utf8("丑"), result.sources[result.ambiguities[0].sourceIndex]);
  EXPECT_EQ(utf8("醜"),
            result.output.substr(result.ambiguities[0].outputOffset,
                                 result.ambiguities[0].outputLength));
}

} // namespace opencc
