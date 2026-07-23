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
#include "DictGroup.hpp"
#include "Lexicon.hpp"
#include "MaxMatchSegmentation.hpp"
#include "PipelineConverter.hpp"
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
  ASSERT_EQ(1u, result.ambiguities.size());
  EXPECT_EQ(1u, result.ambiguities[0].outputOffset);
  EXPECT_EQ(1u, result.ambiguities[0].outputLength);
  ASSERT_EQ(1u, result.sources.size());
  EXPECT_EQ("b", result.sources[result.ambiguities[0].sourceIndex]);
}

TEST_F(ConversionAmbiguitiesTest, SourcesAreDeduplicated) {
  const ConverterPtr converter =
      MakeConverter({MakeDict({{"b", {"y", "z"}}})});
  const AnnotatedConversion result =
      ConvertWithAmbiguities(*converter, "bcb");
  EXPECT_EQ("ycy", result.output);
  ASSERT_EQ(2u, result.ambiguities.size());
  EXPECT_EQ(0u, result.ambiguities[0].outputOffset);
  EXPECT_EQ(2u, result.ambiguities[1].outputOffset);
  ASSERT_EQ(1u, result.sources.size());
  EXPECT_EQ("b", result.sources[0]);
  EXPECT_EQ(0u, result.ambiguities[0].sourceIndex);
  EXPECT_EQ(0u, result.ambiguities[1].sourceIndex);
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
  ASSERT_EQ(1u, result.ambiguities.size());
  EXPECT_EQ(1u, result.ambiguities[0].outputOffset);
  EXPECT_EQ(1u, result.ambiguities[0].outputLength);
  EXPECT_EQ("b", result.sources[result.ambiguities[0].sourceIndex]);
}

TEST_F(ConversionAmbiguitiesTest, LaterStageAmbiguityKeepsMatchPrecision) {
  // Regression test for run coalescing: stage 1 matches nothing in the
  // segment (its runs are all unambiguous), and stage 2 flags a single
  // character in the middle.  The span must stay one character wide -- it
  // must NOT widen to the whole segment.  This mirrors real multi-stage
  // chains (e.g. s2tw), where stage one is a phrase pass that rarely
  // flags anything and the variant pass flags individual characters
  // inside long unsegmented runs.
  std::list<ConversionPtr> conversions{
      ConversionPtr(new Conversion(MakeDict({{"zz", {"zz"}}}))),
      ConversionPtr(new Conversion(MakeDict({{"x", {"m", "n"}}})))};
  const SegmentationPtr segmentation(
      new MaxMatchSegmentation(MakeDict({})));
  // The regression is only observable when the whole input is one segment
  // (per-segment walks reset the run list); guard the precondition so a
  // future segmentation change fails here instead of silently turning
  // this into an empty test.
  ASSERT_EQ(1u, segmentation->Segment("aaaxbbb")->Length());
  const ConverterPtr converter(new SingleStageConverter(
      segmentation, ConversionChainPtr(new ConversionChain(conversions))));
  const AnnotatedConversion result =
      ConvertWithAmbiguities(*converter, "aaaxbbb");
  EXPECT_EQ("aaambbb", result.output);
  EXPECT_EQ(converter->Convert("aaaxbbb"), result.output);
  ASSERT_EQ(1u, result.ambiguities.size());
  EXPECT_EQ(3u, result.ambiguities[0].outputOffset);
  EXPECT_EQ(1u, result.ambiguities[0].outputLength);
  EXPECT_EQ("x", result.sources[result.ambiguities[0].sourceIndex]);
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
  ASSERT_EQ(1u, result.ambiguities.size());
  EXPECT_EQ(0u, result.ambiguities[0].outputOffset);
  EXPECT_EQ(1u, result.ambiguities[0].outputLength);
  EXPECT_EQ("ab", result.sources[result.ambiguities[0].sourceIndex]);
}

TEST_F(ConversionAmbiguitiesTest, UnionPolicyDictGroupMatchesConvert) {
  // A DictGroup constructed directly with the Union policy reports Union
  // via GetMatchPolicy() (honored by Convert()'s PrefixMatch tables), but
  // its inherited MatchPrefix() short-circuits; the walk must apply the
  // policy semantics itself or its output diverges from Convert().
  const DictPtr group(new DictGroup(
      std::list<DictPtr>{MakeDict({{"a", {"x"}}}), MakeDict({{"ab", {"y"}}})},
      DictGroupMatchPolicy::Union));
  std::list<ConversionPtr> conversions{ConversionPtr(new Conversion(group))};
  const ConverterPtr converter(new SingleStageConverter(
      SegmentationPtr(new MaxMatchSegmentation(group)),
      ConversionChainPtr(new ConversionChain(conversions))));
  ASSERT_EQ("y", converter->Convert("ab"));
  const AnnotatedConversion result = ConvertWithAmbiguities(*converter, "ab");
  EXPECT_EQ(converter->Convert("ab"), result.output);
}

TEST_F(ConversionAmbiguitiesTest, UnionPolicyTieBreakMatchesConvert) {
  // Two children share the same key with different values; PrefixMatch's
  // union semantics let the earlier child win the tie, and the walk's
  // recursion (strict > comparison) must agree with Convert().
  const DictPtr group(new DictGroup(
      std::list<DictPtr>{MakeDict({{"ab", {"first"}}}),
                         MakeDict({{"ab", {"second"}}})},
      DictGroupMatchPolicy::Union));
  std::list<ConversionPtr> conversions{ConversionPtr(new Conversion(group))};
  const ConverterPtr converter(new SingleStageConverter(
      SegmentationPtr(new MaxMatchSegmentation(group)),
      ConversionChainPtr(new ConversionChain(conversions))));
  const AnnotatedConversion result = ConvertWithAmbiguities(*converter, "ab");
  EXPECT_EQ(converter->Convert("ab"), result.output);
  EXPECT_EQ("first", result.output);
}

TEST_F(ConversionAmbiguitiesTest, UnionNumValuesComesFromWinningChild) {
  // The flag must reflect NumValues() of exactly the entry the union
  // picked (first child wins same-length ties), not values merged across
  // children.  Uses the UnionDictGroup subclass directly so the override
  // path is covered alongside the plain Union-policy DictGroup tests.
  auto makeConverter = [](const DictPtr& first, const DictPtr& second) {
    const DictPtr group(
        new UnionDictGroup(std::list<DictPtr>{first, second}));
    std::list<ConversionPtr> conversions{
        ConversionPtr(new Conversion(group))};
    return ConverterPtr(new SingleStageConverter(
        SegmentationPtr(new MaxMatchSegmentation(group)),
        ConversionChainPtr(new ConversionChain(conversions))));
  };
  const DictPtr multi = MakeDict({{"ab", {"x", "y"}}});
  const DictPtr single = MakeDict({{"ab", {"z"}}});

  // Multi-value child wins the tie: flagged, default is its first value.
  const AnnotatedConversion flagged =
      ConvertWithAmbiguities(*makeConverter(multi, single), "ab");
  EXPECT_EQ("x", flagged.output);
  ASSERT_EQ(1u, flagged.ambiguities.size());
  EXPECT_EQ("ab", flagged.sources[flagged.ambiguities[0].sourceIndex]);

  // Single-value child wins the tie: NOT flagged, even though another
  // child holds a multi-value entry for the same key.
  const AnnotatedConversion unflagged =
      ConvertWithAmbiguities(*makeConverter(single, multi), "ab");
  EXPECT_EQ("z", unflagged.output);
  EXPECT_TRUE(unflagged.ambiguities.empty());
}

TEST_F(ConversionAmbiguitiesTest, EmptyDictionaryValueDoesNotBreakCompose) {
  // An empty-value entry matching at the end of a segment leaves a
  // trailing zero-width run on the alignment's middle coordinate; Compose
  // must fold it instead of asserting or reading out of bounds, and the
  // output must still match Convert().
  std::list<ConversionPtr> conversions{
      ConversionPtr(
          new Conversion(MakeDict({{"a", {"m", "n"}}, {"b", {""}}}))),
      ConversionPtr(new Conversion(MakeDict({{"m", {"M"}}})))};
  const ConverterPtr converter(new SingleStageConverter(
      SegmentationPtr(new MaxMatchSegmentation(MakeDict({{"ab", {"ab"}}}))),
      ConversionChainPtr(new ConversionChain(conversions))));
  const AnnotatedConversion result = ConvertWithAmbiguities(*converter, "ab");
  EXPECT_EQ(converter->Convert("ab"), result.output);
  EXPECT_EQ("M", result.output);
  ASSERT_EQ(1u, result.ambiguities.size());
  // The trailing fold widens the span's source to cover the vanished tail.
  EXPECT_EQ("ab", result.sources[result.ambiguities[0].sourceIndex]);
}

TEST_F(ConversionAmbiguitiesTest, NulBytePreservedWithoutSegmentation) {
  // Without a segmentation step, Convert() processes the whole text as one
  // string_view, preserving embedded NUL bytes; the walk must do the same
  // instead of truncating at the first NUL.
  std::list<ConversionPtr> conversions{
      ConversionPtr(new Conversion(MakeDict({{"b", {"y", "z"}}})))};
  const ConverterPtr converter(new SingleStageConverter(
      nullptr, ConversionChainPtr(new ConversionChain(conversions))));
  const std::string input("a\0b", 3);
  const std::string expected = converter->Convert(input);
  ASSERT_EQ(std::string("a\0y", 3), expected);
  const AnnotatedConversion result = ConvertWithAmbiguities(*converter, input);
  EXPECT_EQ(expected, result.output);
  ASSERT_EQ(1u, result.ambiguities.size());
  EXPECT_EQ(2u, result.ambiguities[0].outputOffset);
  EXPECT_EQ("b", result.sources[result.ambiguities[0].sourceIndex]);
}

TEST_F(ConversionAmbiguitiesTest, StreamChunkCarriesAnalyzedFlag) {
  const ConverterPtr pipeline(new PipelineConverter(
      {MakeConverter({MakeDict({{"b", {"y", "z"}}})})}));
  AmbiguityStream unanalyzed(pipeline);
  EXPECT_FALSE(unanalyzed.Finish("b").analyzed);

  AmbiguityStream analyzed(MakeConverter({MakeDict({{"b", {"y", "z"}}})}));
  EXPECT_TRUE(analyzed.Finish("b").analyzed);
}

TEST_F(ConversionAmbiguitiesTest, PipelineConverterIsFlaggedUnanalyzed) {
  // A converter without a single conversion chain still converts, but the
  // result must be distinguishable from "converted with no ambiguities".
  const ConverterPtr pipeline(new PipelineConverter(
      {MakeConverter({MakeDict({{"b", {"y", "z"}}})})}));
  const AnnotatedConversion result = ConvertWithAmbiguities(*pipeline, "b");
  EXPECT_EQ("y", result.output);
  EXPECT_FALSE(result.analyzed);
  EXPECT_TRUE(result.ambiguities.empty());

  const AnnotatedConversion analyzed = ConvertWithAmbiguities(
      *MakeConverter({MakeDict({{"b", {"y", "z"}}})}), "b");
  EXPECT_TRUE(analyzed.analyzed);
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
  ASSERT_EQ(1u, result.ambiguities.size());
  EXPECT_EQ(2u, result.ambiguities[0].outputOffset);
  EXPECT_EQ("b", result.sources[result.ambiguities[0].sourceIndex]);
}

TEST_F(ConversionAmbiguitiesTest, ChunkedStreamMatchesWholeInput) {
  // Two stages with phrase (multi-character) entries, so the test also
  // covers segmentation stability across flush-window boundaries.
  const ConverterPtr converter =
      MakeConverter({MakeDict({{"ab", {"XY"}},
                               {"b", {"y", "z"}},
                               {"c", {"p", "q"}},
                               {"de", {"UV", "WW"}}}),
                     MakeDict({{"XY", {"J"}}, {"p", {"r", "s"}}})});
  std::string input;
  for (int i = 0; i < 100; i++) {
    input += "abcabde";
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

TEST_F(ConversionAmbiguitiesTest, DefaultWindowingMatchesConverterStream) {
  // AmbiguityStream's default keep-tail window must equal
  // ConverterStream's, or the two wrappers flush on different boundaries
  // and stream/whole-input results diverge on long entries.  The constant
  // lives in a private header and cannot reference the installed one, so
  // pin the two defaults together behaviorally: identical chunked input
  // must produce identical per-chunk flush output.
  const ConverterPtr converter =
      MakeConverter({MakeDict({{"ab", {"XY"}}, {"b", {"y", "z"}}})});
  std::string input;
  for (int i = 0; i < 100; i++) {
    input += "abcab";
  }
  ConverterStream reference(converter);
  AmbiguityStream stream(converter);
  for (size_t pos = 0; pos < input.size(); pos += 7) {
    const std::string_view chunk = std::string_view(input).substr(pos, 7);
    EXPECT_EQ(reference.ConvertChunk(chunk),
              stream.ConvertChunk(chunk).output);
  }
  EXPECT_EQ(reference.Finish(), stream.Finish().output);
}

TEST_F(ConversionAmbiguitiesTest, Utf8OffsetsAreBytes) {
  const ConverterPtr converter = MakeConverter(
      {MakeDict({{utf8("丑"), {utf8("醜"), utf8("丑")}}})});
  const std::string input = utf8("文丑");
  const AnnotatedConversion result = ConvertWithAmbiguities(*converter, input);
  EXPECT_EQ(utf8("文醜"), result.output);
  ASSERT_EQ(1u, result.ambiguities.size());
  EXPECT_EQ(3u, result.ambiguities[0].outputOffset);
  EXPECT_EQ(3u, result.ambiguities[0].outputLength);
  EXPECT_EQ(utf8("丑"), result.sources[result.ambiguities[0].sourceIndex]);
  EXPECT_EQ(utf8("醜"),
            result.output.substr(result.ambiguities[0].outputOffset,
                                 result.ambiguities[0].outputLength));
}

} // namespace opencc
