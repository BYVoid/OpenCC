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

#include "ConversionChain.hpp"
#include "Converter.hpp"
#include "DictGroupTestBase.hpp"
#include "MaxMatchSegmentation.hpp"

namespace opencc {

class ConversionChainTest : public DictGroupTestBase {
protected:
  ConversionChainTest() {}

  virtual void SetUp() {
    dict = CreateDictGroupForConversion();
    conversion = ConversionPtr(new Conversion(dict));
  }

  void SegmentsAssertEquals(const SegmentsPtr& expected,
                            const SegmentsPtr& actual) {
    const size_t length = expected->Length();
    EXPECT_TRUE(length == actual->Length());
    for (size_t i = 0; i < length; i++) {
      EXPECT_EQ(std::string(expected->At(i)), std::string(actual->At(i)))
          << "i = " << i;
    }
  }

  DictPtr dict;
  ConversionPtr conversion;
};

TEST_F(ConversionChainTest, Convert) {
  // Variants
  const DictPtr& dictVariants = CreateDictForTaiwanVariants();
  const ConversionPtr& conversionVariants =
      ConversionPtr(new Conversion(dictVariants));
  const std::list<ConversionPtr> conversions{conversion, conversionVariants};
  const ConversionChainPtr& conversionChain =
      ConversionChainPtr(new ConversionChain(conversions));
  const SegmentsPtr& converted =
      conversionChain->Convert(SegmentsPtr(new Segments{utf8("里面")}));
  SegmentsAssertEquals(SegmentsPtr(new Segments{utf8("裡面")}), converted);
}

TEST_F(ConversionChainTest, StreamKeepsIncompleteIdeographicDescriptionSequence) {
  LexiconPtr lexicon(new Lexicon);
  lexicon->Add(DictEntryFactory::New(utf8("钅"), utf8("釒")));
  lexicon->Add(DictEntryFactory::New(utf8("只"), utf8("隻")));
  lexicon->Sort();

  DictPtr componentDict(new TextDict(lexicon));
  SegmentationPtr segmentation(new MaxMatchSegmentation(componentDict));
  ConversionPtr componentConversion(new Conversion(componentDict));
  ConversionChainPtr chain(
      new ConversionChain(std::list<ConversionPtr>{componentConversion}));
  ConverterPtr converter(new SingleStageConverter(segmentation, chain));
  ConverterStream stream(converter, 1);

  const std::string firstChunk = utf8("prefix⿰钅");
  const std::string secondChunk = utf8("只只");
  std::string output;
  output += stream.ConvertChunk(firstChunk);
  output += stream.Finish(secondChunk);

  EXPECT_EQ(utf8("prefix⿰钅只隻"), output);
}

TEST_F(ConversionChainTest,
       StreamKeepsIncompleteIdeographicDescriptionSequenceBeforeKeepLimit) {
  LexiconPtr lexicon(new Lexicon);
  lexicon->Add(DictEntryFactory::New(utf8("只"), utf8("隻")));
  lexicon->Sort();

  DictPtr componentDict(new TextDict(lexicon));
  SegmentationPtr segmentation(new MaxMatchSegmentation(componentDict));
  ConversionPtr componentConversion(new Conversion(componentDict));
  ConversionChainPtr chain(
      new ConversionChain(std::list<ConversionPtr>{componentConversion}));
  ConverterPtr converter(new SingleStageConverter(segmentation, chain));
  ConverterStream stream(converter, 16);

  std::string nestedPrefix = utf8("prefix");
  std::string expected = utf8("prefix");
  for (size_t i = 0; i < 10; i++) {
    nestedPrefix += utf8("⿰");
    expected += utf8("⿰");
  }
  for (size_t i = 0; i < 10; i++) {
    nestedPrefix += utf8("木");
    expected += utf8("木");
  }
  expected += utf8("只隻");

  const std::string secondChunk = utf8("只只");
  std::string output;
  output += stream.ConvertChunk(nestedPrefix);
  output += stream.Finish(secondChunk);

  EXPECT_EQ(expected, output);
}

class PipelineConverterTest : public TextDictTestBase {
protected:
  // stage1: 钅 → 釒   stage2: 釒 → 金
  void SetUp() override {
    LexiconPtr lex1(new Lexicon);
    lex1->Add(DictEntryFactory::New(utf8("钅"), utf8("釒")));
    lex1->Sort();
    DictPtr dict1(new TextDict(lex1));
    stage1 = ConverterPtr(new SingleStageConverter(
        SegmentationPtr(new MaxMatchSegmentation(dict1)),
        ConversionChainPtr(
            new ConversionChain({ConversionPtr(new Conversion(dict1))}))));
    seg1 = stage1->GetSegmentation();

    LexiconPtr lex2(new Lexicon);
    lex2->Add(DictEntryFactory::New(utf8("釒"), utf8("金")));
    lex2->Sort();
    DictPtr dict2(new TextDict(lex2));
    stage2 = ConverterPtr(new SingleStageConverter(
        SegmentationPtr(new MaxMatchSegmentation(dict2)),
        ConversionChainPtr(
            new ConversionChain({ConversionPtr(new Conversion(dict2))}))));
    seg2 = stage2->GetSegmentation();
  }

  ConverterPtr stage1, stage2;
  SegmentationPtr seg1, seg2;
};

TEST_F(PipelineConverterTest, ConvertChainsStagesInOrder) {
  PipelineConverter pipeline({stage1, stage2});
  EXPECT_EQ(utf8("金"), pipeline.Convert(utf8("钅")));
}

TEST_F(PipelineConverterTest, ConvertEmptyPipelineIsIdentity) {
  PipelineConverter pipeline({});
  EXPECT_EQ(utf8("钅"), pipeline.Convert(utf8("钅")));
}

TEST_F(PipelineConverterTest, GetSegmentationReturnsLastStage) {
  PipelineConverter pipeline({stage1, stage2});
  EXPECT_EQ(seg2, pipeline.GetSegmentation());
}

TEST_F(PipelineConverterTest, GetSegmentationEmptyPipelineReturnsNullptr) {
  PipelineConverter pipeline({});
  EXPECT_EQ(nullptr, pipeline.GetSegmentation());
}

TEST_F(PipelineConverterTest, InspectReturnsInputOutputWithNoStageDetail) {
  PipelineConverter pipeline({stage1, stage2});
  const ConversionInspectionResult result = pipeline.Inspect(utf8("钅"));
  EXPECT_EQ(utf8("钅"), result.input);
  EXPECT_EQ(utf8("金"), result.output);
  EXPECT_TRUE(result.segments.empty());
  EXPECT_TRUE(result.stages.empty());
}

} // namespace opencc
