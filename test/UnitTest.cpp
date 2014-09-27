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

#include <thread>

#include "Config.hpp"
#include "DictTestUtils.hpp"
#include "DictEntry.hpp"
#include "MaxMatchSegmentation.hpp"
#include "ConversionChain.hpp"
#include "Converter.hpp"
#include "opencc.h"

using namespace opencc;

void TestTextDict() {
  TextDictPtr textDict = DictTestUtils::CreateTextDictForText();
  DictTestUtils::TestDict(textDict);
  
  // Serialization
  string fileName = "dict.txt";
  textDict->opencc::SerializableDict::SerializeToFile(fileName);
  
  // Deserialization
  textDict->opencc::SerializableDict::LoadFromFile(fileName);
  DictTestUtils::TestDict(textDict);
}

void TestDartsDict() {
  TextDictPtr textDict = DictTestUtils::CreateTextDictForText();
  DartsDictPtr dartsDict(new DartsDict());
  dartsDict->LoadFromDict(textDict.get());
  DictTestUtils::TestDict(dartsDict);
  
  // Serialization
  string fileName = "dict.ocd";
  dartsDict->opencc::SerializableDict::SerializeToFile(fileName);
  
  // Deserialization
  dartsDict->opencc::SerializableDict::LoadFromFile(fileName);
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
  AssertEquals(utf8("太后"), segments->at(0));
  AssertEquals(utf8("的"), segments->at(1));
  AssertEquals(utf8("头发"), segments->at(2));
  AssertEquals(utf8("干燥"), segments->at(3));
}

void TestConversion() {
  auto dict = DictTestUtils::CreateDictGroupForConversion();
  auto conversion = ConversionPtr(new Conversion(dict));
  string converted = conversion->Convert(utf8("太后的头发干燥"));
  AssertEquals(utf8("太后的頭髮乾燥"), converted);
}

void TestConversionChain() {
  // Dict
  auto dict = DictTestUtils::CreateDictGroupForConversion();
  auto conversion = ConversionPtr(new Conversion(dict));
  // Variants
  auto dictVariants = DictTestUtils::CreateDictForTaiwanVariants();
  auto conversionVariants = ConversionPtr(new Conversion(dictVariants));
  auto conversionChain = ConversionChainPtr(new ConversionChain());
  conversionChain->AddConversion(conversion);
  conversionChain->AddConversion(conversionVariants);
  StringVectorPtr converted = conversionChain->Convert(StringVectorPtr(new StringVector{utf8("里面")}));
  VectorAssertEquals(StringVectorPtr(new StringVector{utf8("裡面")}), converted);
}

const string CONFIG_TEST_PATH = "config_test/config_test.json";

void TestConfig() {
  Config config;
  config.LoadFile(CONFIG_TEST_PATH);
  auto converter = config.GetConverter();
  string converted = converter->Convert(utf8("燕燕于飞差池其羽之子于归远送于野"));
  AssertEquals(utf8("燕燕于飛差池其羽之子于歸遠送於野"), converted);
}

void TestMultithreading() {
  auto routine = [](std::string name) {
    SimpleConverter converter(name);
    string converted = converter.Convert(utf8("燕燕于飞差池其羽之子于归远送于野"));
    AssertEquals(utf8("燕燕于飛差池其羽之子于歸遠送於野"), converted);
  };
  std::thread thread1(routine, CONFIG_TEST_PATH);
  std::thread thread2(routine, CONFIG_TEST_PATH);
  routine(CONFIG_TEST_PATH);
  thread1.join();
  thread2.join();
}

int main(int argc, const char * argv[]) {
  TestUtils::RunTest("TestTextDict", TestTextDict);
  TestUtils::RunTest("TestDartsDict", TestDartsDict);
  TestUtils::RunTest("TestDictGroup", TestDictGroup);
  TestUtils::RunTest("TestSegmentation", TestSegmentation);
  TestUtils::RunTest("TestConversion", TestConversion);
  TestUtils::RunTest("TestConversionChain", TestConversionChain);
  TestUtils::RunTest("TestConfig", TestConfig);
  TestUtils::RunTest("TestMultithreading", TestMultithreading);
}
