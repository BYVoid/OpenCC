/*
 * Open Chinese Convert
 *
 * Copyright 2010-2013 BYVoid <byvoid@byvoid.com>
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

#include "Config.hpp"
#include "DictTestUtils.hpp"
#include "DictEntry.hpp"
#include "MaxMatchSegmentation.hpp"
#include "ConversionChain.hpp"

using namespace Opencc;

void TestTextDict() {
  TextDictPtr textDict = DictTestUtils::CreateTextDictForText();
  DictTestUtils::TestDict(textDict);
  
  // Serialization
  string fileName = "dict.txt";
  textDict->Opencc::SerializableDict::SerializeToFile(fileName);
  
  // Deserialization
  textDict->Opencc::SerializableDict::LoadFromFile(fileName);
  DictTestUtils::TestDict(textDict);
}

void TestDartsDict() {
  TextDictPtr textDict = DictTestUtils::CreateTextDictForText();
  DartsDictPtr dartsDict(new DartsDict());
  dartsDict->LoadFromDict(textDict.get());
  DictTestUtils::TestDict(dartsDict);
  
  // Serialization
  string fileName = "dict.ocd";
  dartsDict->Opencc::SerializableDict::SerializeToFile(fileName);
  
  // Deserialization
  dartsDict->Opencc::SerializableDict::LoadFromFile(fileName);
  DictTestUtils::TestDict(dartsDict);
}

void TestDictGroup() {
  auto dictGroup = DictTestUtils::CreateDictGroupForConversion();
  Optional<DictEntryPtr> entry;
  entry = dictGroup->MatchPrefix("Unknown");
  AssertTrue(entry.IsNull());
  
  DictEntryPtrVectorPtr matches = dictGroup->MatchAllPrefixes(utf8("干燥"));
  AssertEquals(2, matches->size());
  AssertEquals(utf8("乾燥"), matches->at(0)->GetDefault());
  AssertEquals(utf8("幹"), matches->at(1)->GetDefault());
}

void TestSegmentation() {
  auto dict = DictTestUtils::CreateDictGroupForConversion();
  auto segmentation = SegmentationPtr(new MaxMatchSegmentation(dict));
  auto segments = segmentation->Segment(utf8("太后的头发干燥"));
  AssertEquals(4, segments->size());
  AssertEquals(utf8("太后"), segments->at(0)->key);
  AssertEquals(utf8("太后"), segments->at(0)->GetDefault());
  AssertEquals(utf8("的"), segments->at(1)->key);
  AssertEquals(utf8("的"), segments->at(1)->GetDefault());
  AssertEquals(utf8("头发"), segments->at(2)->key);
  AssertEquals(utf8("頭髮"), segments->at(2)->GetDefault());
  AssertEquals(utf8("干燥"), segments->at(3)->key);
  AssertEquals(utf8("乾燥"), segments->at(3)->GetDefault());
}

void TestConversion() {
  auto dict = DictTestUtils::CreateDictGroupForConversion();
  auto segmentation = SegmentationPtr(new MaxMatchSegmentation(dict));
  auto conversion = ConversionPtr(new Conversion(segmentation));
  string converted = conversion->Convert(utf8("太后的头发干燥"));
  AssertEquals(utf8("太后的頭髮乾燥"), converted);
}

void TestConversionChain() {
  // Dict
  auto dict = DictTestUtils::CreateDictGroupForConversion();
  auto segmentation = SegmentationPtr(new MaxMatchSegmentation(dict));
  auto conversion = ConversionPtr(new Conversion(segmentation));
  // Variants
  auto dictVariants = DictTestUtils::CreateDictForTaiwanVariants();
  auto conversionVariants = ConversionPtr(new Conversion(SegmentationPtr(new MaxMatchSegmentation(dictVariants))));
  auto conversionChain = ConversionChainPtr(new ConversionChain());
  conversionChain->AddConversion(conversion);
  conversionChain->AddConversion(conversionVariants);
  string converted = conversionChain->Convert(utf8("里面"));
  AssertEquals(utf8("裡面"), converted);
}

void TestConfig() {
  Config config;
  config.LoadFile("config_test/config_test.json");
  auto conversionChain = config.GetConversionChain();
  string converted = conversionChain->Convert(utf8("燕燕于飞差池其羽之子于归远送于野"));
  AssertEquals(utf8("燕燕于飛差池其羽之子于歸遠送於野"), converted);
}

int main(int argc, const char * argv[]) {
  TestUtils::RunTest("TestTextDict", TestTextDict);
  TestUtils::RunTest("TestDartsDict", TestDartsDict);
  TestUtils::RunTest("TestDictGroup", TestDictGroup);
  TestUtils::RunTest("TestSegmentation", TestSegmentation);
  TestUtils::RunTest("TestConversion", TestConversion);
  TestUtils::RunTest("TestConversionChain", TestConversionChain);
  TestUtils::RunTest("TestConfig", TestConfig);
}
