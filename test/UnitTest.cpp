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

#include "BinaryDict.hpp"
#include "Config.hpp"
#include "ConversionChain.hpp"
#include "Converter.hpp"
#include "DictTestUtils.hpp"
#include "MaxMatchSegmentation.hpp"
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

void TestBinaryDict() {
  TextDictPtr textDict = DictTestUtils::CreateTextDictForText();
  BinaryDictPtr binDict(new BinaryDict(textDict->GetLexicon()));

  // Serialization
  string fileName = "dict.bin";
  binDict->opencc::SerializableDict::SerializeToFile(fileName);

  // Deserialization
  BinaryDictPtr deserialized = SerializableDict::NewFromFile<BinaryDict>(fileName);
  const LexiconPtr& lex1 = binDict->GetLexicon();
  const LexiconPtr& lex2 = deserialized->GetLexicon();

  AssertEquals(lex1->Length(), lex2->Length());
  for (size_t i = 0; i < lex1->Length(); i++) {
    AssertEquals(string(lex1->At(i)->Key()), lex2->At(i)->Key());
    AssertEquals(lex1->At(i)->NumValues(), lex2->At(i)->NumValues());
  }

  TextDictPtr deserializedTextDict(new TextDict(lex2));
  DictTestUtils::TestDict(deserializedTextDict);
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
  AssertEquals(4, segments->Length());
  AssertEquals(utf8("太后"), string(segments->At(0)));
  AssertEquals(utf8("的"), string(segments->At(1)));
  AssertEquals(utf8("头发"), string(segments->At(2)));
  AssertEquals(utf8("干燥"), string(segments->At(3)));
}

void TestConversion() {
  auto dict = DictTestUtils::CreateDictGroupForConversion();
  auto conversion = ConversionPtr(new Conversion(dict));
  const string& input = utf8("太后的头发干燥");
  const string& expected = utf8("太后的頭髮乾燥");
  {
    string converted = conversion->Convert(input);
    AssertEquals(expected, converted);
  }
  {
    string converted = conversion->Convert(input.c_str());
    AssertEquals(expected, converted);
  }
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
  auto converted = conversionChain->Convert(
      SegmentsPtr(new Segments{utf8("里面")}));
  SegmentsAssertEquals(SegmentsPtr(new Segments{utf8("裡面")}), converted);
}

const string CONFIG_TEST_PATH = "config_test/config_test.json";

void TestConfigConverter() {
  Config config;
  auto converter = config.NewFromFile(CONFIG_TEST_PATH);
  const string& input = utf8("燕燕于飞差池其羽之子于归远送于野");
  const string& expected = utf8("燕燕于飛差池其羽之子于歸遠送於野");
  {
    string converted = converter->Convert(input);
    AssertEquals(expected, converted);
  }
  {
    char output[1024];
    size_t length = converter->Convert(input.c_str(), output);
    AssertEquals(expected.length(), length);
    AssertEquals(expected, output);
  }
  {
    string path = "/opencc/no/such/file/or/directory";
    try {
      auto converter = config.NewFromFile(path);
    } catch (FileNotFound& e) {
      AssertEquals(path + " not found or not accessible.", e.what());
    }
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
  {
    opencc_t od = opencc_open(CONFIG_TEST_PATH.c_str());
    char* converted = opencc_convert_utf8(od, text.c_str(), (size_t)-1);
    AssertEquals(expected, converted);
    opencc_convert_utf8_free(converted);
    AssertEquals(0, opencc_close(od));
  }
  {
    char output[1024];
    opencc_t od = opencc_open(CONFIG_TEST_PATH.c_str());
    size_t length = opencc_convert_utf8_to_buffer(od, text.c_str(), (size_t)-1,
                                                  output);
    AssertEquals(expected.length(), length);
    AssertEquals(expected, output);
    AssertEquals(0, opencc_close(od));
  }
  {
    string path = "/opencc/no/such/file/or/directory";
    opencc_t od = opencc_open(path.c_str());
    AssertEquals(reinterpret_cast<opencc_t>(-1), od);
    AssertEquals(path + " not found or not accessible.", opencc_error());
  }
}

int main(int argc, const char* argv[]) {
  TestUtils::RunTest("TestTextDict", TestTextDict);
  TestUtils::RunTest("TestBinaryDict", TestBinaryDict);
  TestUtils::RunTest("TestDartsDict", TestDartsDict);
  TestUtils::RunTest("TestDictGroup", TestDictGroup);
  TestUtils::RunTest("TestSegmentation", TestSegmentation);
  TestUtils::RunTest("TestConversion", TestConversion);
  TestUtils::RunTest("TestConversionChain", TestConversionChain);
  TestUtils::RunTest("TestConfigConverter", TestConfigConverter);
  TestUtils::RunTest("TestMultithreading", TestMultithreading);
  TestUtils::RunTest("TestCInterface", TestCInterface);
}
