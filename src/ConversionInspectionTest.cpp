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

#include "ConfigTestBase.hpp"
#include "ConversionInspection.hpp"
#include "Segments.hpp"
#include "SimpleConverter.hpp"
#include "TestUtilsUTF8.hpp"

namespace opencc {

// ---- Segments::ToVector tests ----

class SegmentsToVectorTest : public ::testing::Test {};

TEST_F(SegmentsToVectorTest, ManagedSegments) {
  Segments segs;
  segs.AddSegment(std::string("hello"));
  segs.AddSegment(std::string("world"));
  const auto vec = segs.ToVector();
  ASSERT_EQ(2u, vec.size());
  EXPECT_EQ("hello", vec[0]);
  EXPECT_EQ("world", vec[1]);
}

TEST_F(SegmentsToVectorTest, UnmanagedSegments) {
  // Unmanaged segments use const char* directly
  const char* a = "foo";
  const char* b = "bar";
  Segments segs;
  segs.AddSegment(a);
  segs.AddSegment(b);
  const auto vec = segs.ToVector();
  ASSERT_EQ(2u, vec.size());
  EXPECT_EQ("foo", vec[0]);
  EXPECT_EQ("bar", vec[1]);
}

TEST_F(SegmentsToVectorTest, MixedSegments) {
  const char* a = "unmanaged";
  Segments segs;
  segs.AddSegment(a);
  segs.AddSegment(std::string("managed"));
  segs.AddSegment(a);
  const auto vec = segs.ToVector();
  ASSERT_EQ(3u, vec.size());
  EXPECT_EQ("unmanaged", vec[0]);
  EXPECT_EQ("managed", vec[1]);
  EXPECT_EQ("unmanaged", vec[2]);
}

TEST_F(SegmentsToVectorTest, EmptySegments) {
  Segments segs;
  const auto vec = segs.ToVector();
  EXPECT_TRUE(vec.empty());
}

// ---- SimpleConverter::Inspect / Converter::Inspect tests ----

class ConversionInspectionTest : public ConfigTestBase {
protected:
  ConversionInspectionTest() {}
};

TEST_F(ConversionInspectionTest, OutputMatchesConvert) {
  const SimpleConverter converter(CONFIG_TEST_JSON_PATH);
  const std::string text = utf8("燕燕于飞差池其羽之子于归远送于野");
  const ConversionInspectionResult result = converter.Inspect(text);
  const std::string converted = converter.Convert(text);
  EXPECT_EQ(converted, result.output);
}

TEST_F(ConversionInspectionTest, InputPreserved) {
  const SimpleConverter converter(CONFIG_TEST_JSON_PATH);
  const std::string text = utf8("燕燕于飞差池其羽之子于归远送于野");
  const ConversionInspectionResult result = converter.Inspect(text);
  EXPECT_EQ(text, result.input);
}

TEST_F(ConversionInspectionTest, SegmentsNonEmpty) {
  const SimpleConverter converter(CONFIG_TEST_JSON_PATH);
  const std::string text = utf8("燕燕于飞差池其羽之子于归远送于野");
  const ConversionInspectionResult result = converter.Inspect(text);
  EXPECT_FALSE(result.segments.empty());
  // The segments concatenated should equal the original input
  std::string joined;
  for (const auto& s : result.segments) {
    joined += s;
  }
  EXPECT_EQ(text, joined);
}

TEST_F(ConversionInspectionTest, StagesCountMatchesChain) {
  // The config_test.json has one conversion chain entry -> 1 stage
  const SimpleConverter converter(CONFIG_TEST_JSON_PATH);
  const std::string text = utf8("燕燕于飞差池其羽之子于归远送于野");
  const ConversionInspectionResult result = converter.Inspect(text);
  EXPECT_EQ(1u, result.stages.size());
}

TEST_F(ConversionInspectionTest, StageIndexStartsAtOne) {
  const SimpleConverter converter(CONFIG_TEST_JSON_PATH);
  const std::string text = utf8("燕燕于飞差池其羽之子于归远送于野");
  const ConversionInspectionResult result = converter.Inspect(text);
  ASSERT_FALSE(result.stages.empty());
  EXPECT_EQ(1u, result.stages[0].index);
}

TEST_F(ConversionInspectionTest, EmptyInput) {
  const SimpleConverter converter(CONFIG_TEST_JSON_PATH);
  const ConversionInspectionResult result = converter.Inspect("");
  EXPECT_EQ("", result.input);
  EXPECT_EQ("", result.output);
  // segments should be empty or contain one empty segment
  std::string joined;
  for (const auto& s : result.segments) {
    joined += s;
  }
  EXPECT_EQ("", joined);
}

} // namespace opencc
