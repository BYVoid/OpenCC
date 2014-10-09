/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 BYVoid <byvoid@byvoid.com>
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
#include "ConversionChain.hpp"
#include "Converter.hpp"
#include "DictEntry.hpp"
#include "DictTestUtils.hpp"
#include "MaxMatchSegmentation.hpp"
#include "Segments.hpp"
#include "opencc.h"

using namespace opencc;

void TestTextDict() {
  TextDictPtr textDict = DictTestUtils::CreateTextDictForText();
  DictTestUtils::TestDict(textDict);

  // Serialization
  string fileName = "dict.txt";
  textDict->opencc::SerializableDict::SerializeToFile(fileName);

  // Deserialization
  TextDictPtr deserialized = SerializableDict::NewFromFile<TextDict>(fileName);
  DictTestUtils::TestDict(deserialized);
}

void TestDartsDict() {
  TextDictPtr textDict = DictTestUtils::CreateTextDictForText();
  DartsDictPtr dartsDict = DartsDict::NewFromDict(*textDict.get());
  DictTestUtils::TestDict(dartsDict);

  // Serialization
  string fileName = "dict.ocd";
  dartsDict->opencc::SerializableDict::SerializeToFile(fileName);

  // Deserialization
  DartsDictPtr deserialized = SerializableDict::NewFromFile<DartsDict>(fileName);
  DictTestUtils::TestDict(deserialized);
}

void TestDictGroup() {
  {
    const auto& dictGroup = DictTestUtils::CreateDictGroupForConversion();
    const auto& entry = dictGroup->Dict::MatchPrefix(utf8("Unknown"));
    AssertTrue(entry.IsNull());

    const auto& matches = dictGroup->Dict::MatchAllPrefixes(utf8("干燥"));
    AssertEquals(2, matches.size());
    AssertEquals(utf8("乾燥"), matches.at(0)->GetDefault());
    AssertEquals(utf8("幹"), matches.at(1)->GetDefault());
  }
  {
    DictGroupPtr dictGroup(new DictGroup(list<DictPtr>{
        DictTestUtils::CreateDictForPhrases(),
        DictTestUtils::CreateTaiwanPhraseDict()
    }));
    {
      const auto& entry = dictGroup->Dict::MatchPrefix(utf8("鼠标"));
      AssertEquals(utf8("鼠標"), entry.Get()->GetDefault());
    }
    {
      const auto& entry = dictGroup->Dict::MatchPrefix(utf8("克罗地亚"));
      AssertEquals(utf8("克羅埃西亞"), entry.Get()->GetDefault());
    }
    {
      const auto& matches = dictGroup->Dict::MatchAllPrefixes(utf8("鼠标"));
      AssertEquals(1, matches.size());
      AssertEquals(utf8("鼠標"), matches[0]->GetDefault());
    }
  }
}

void TestSegmentation() {
  auto dict = DictTestUtils::CreateDictGroupForConversion();
  auto segmentation = SegmentationPtr(new MaxMatchSegmentation(dict));
  const auto& segments = segmentation->Segment(utf8("太后的头发干燥"));
  AssertEquals(4, segments.Length());
  AssertEquals(utf8("太后"), string(segments.At(0)));
  AssertEquals(utf8("的"), string(segments.At(1)));
  AssertEquals(utf8("头发"), string(segments.At(2)));
  AssertEquals(utf8("干燥"), string(segments.At(3)));
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
  list<ConversionPtr> conversions;
  conversions.push_back(conversion);
  conversions.push_back(conversionVariants);
  auto conversionChain = ConversionChainPtr(new ConversionChain(conversions));
  auto converted = conversionChain->Convert(Segments{utf8("里面")});
  SegmentsAssertEquals(Segments{utf8("裡面")}, converted);
}

const string CONFIG_TEST_PATH = "config_test/config_test.json";

void TestConfig() {
  Config config;
  auto converter = config.NewFromFile(CONFIG_TEST_PATH);
  string converted = converter->Convert(utf8("燕燕于飞差池其羽之子于归远送于野"));
  AssertEquals(utf8("燕燕于飛差池其羽之子于歸遠送於野"), converted);

  string path = "/opencc/no/such/file/or/directory";
  try {
    auto converter = config.NewFromFile(path);
  } catch (FileNotFound& e) {
    AssertEquals(path + " not found or not accessible.", e.what());
  }
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

void TestCInterface() {
  const string& text = utf8("燕燕于飞差池其羽之子于归远送于野");
  const string& expected = utf8("燕燕于飛差池其羽之子于歸遠送於野");
  opencc_t od = opencc_new(CONFIG_TEST_PATH.c_str());
  char* converted = opencc_convert(od, text.c_str());
  AssertEquals(expected, converted);
  opencc_free_string(converted);
  opencc_delete(od);
}

int main(int argc, const char* argv[]) {
  TestUtils::RunTest("TestTextDict", TestTextDict);
  TestUtils::RunTest("TestDartsDict", TestDartsDict);
  TestUtils::RunTest("TestDictGroup", TestDictGroup);
  TestUtils::RunTest("TestSegmentation", TestSegmentation);
  TestUtils::RunTest("TestConversion", TestConversion);
  TestUtils::RunTest("TestConversionChain", TestConversionChain);
  TestUtils::RunTest("TestConfig", TestConfig);
  TestUtils::RunTest("TestMultithreading", TestMultithreading);
  TestUtils::RunTest("TestCInterface", TestCInterface);
}
