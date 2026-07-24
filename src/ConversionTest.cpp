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

#include <random>

#include "Conversion.hpp"
#include "DictGroupTestBase.hpp"
#include "MarisaDict.hpp"
#include "PrefixMatch.hpp"
#include "UTF8Util.hpp"

namespace opencc {

namespace {

// Reference implementation of the conversion loop that advances one
// character (or IDS sequence) per unmatched position and never bulk-skips.
// This is the pre-skip-scan behavior; Conversion::Convert must stay
// byte-for-byte equivalent to it on any input, including malformed UTF-8.
std::string ReferenceConvert(const PrefixMatch& prefixMatch,
                             const std::string& input) {
  std::string output;
  const char* pstr = input.data();
  const char* end = pstr + input.size();
  while (pstr < end) {
    const size_t remaining = end - pstr;
    const PrefixMatchView matched = prefixMatch.MatchPrefixView(pstr, remaining);
    size_t matchedLength;
    if (!matched.matched) {
      matchedLength =
          UTF8Util::NextIdeographicDescriptionSequenceLength(pstr, remaining);
      if (matchedLength == 0) {
        matchedLength = UTF8Util::NextCharLength(pstr);
      }
      if (matchedLength > remaining) {
        matchedLength = remaining;
      }
      output.append(pstr, matchedLength);
    } else {
      matchedLength = matched.keyLength;
      if (matchedLength > remaining) {
        matchedLength = remaining;
      }
      output.append(matched.value.data(), matched.value.size());
    }
    pstr += matchedLength;
  }
  return output;
}

} // namespace

class ConversionTest : public DictGroupTestBase {
protected:
  ConversionTest()
      : input(utf8("太后的头发干燥")), expected(utf8("太后的頭髮乾燥")) {}

  virtual void SetUp() {
    dict = CreateDictGroupForConversion();
    conversion = ConversionPtr(new Conversion(dict));
  }

  DictPtr dict;
  ConversionPtr conversion;
  const std::string input;
  const std::string expected;
};

TEST_F(ConversionTest, ConvertString) {
  const std::string converted = conversion->Convert(input);
  EXPECT_EQ(expected, converted);
}

TEST_F(ConversionTest, ConvertCString) {
  const std::string converted = conversion->Convert(input.c_str());
  EXPECT_EQ(expected, converted);
}

TEST_F(ConversionTest, ConvertMixedAsciiAndChinese) {
  // Long ASCII runs exercise the vectorized skip of unmatchable characters.
  const std::string asciiPrefix =
      "The quick brown fox jumps over the lazy dog 0123456789 ";
  const std::string asciiSuffix = " <html attr=\"value\">tail</html>";
  const std::string punctuation = utf8("、「」");
  EXPECT_EQ(asciiPrefix + expected + punctuation + expected + asciiSuffix,
            conversion->Convert(asciiPrefix + input + punctuation + input +
                                asciiSuffix));
}

TEST_F(ConversionTest, PreserveDictGroupPriority) {
  LexiconPtr firstLexicon(new Lexicon);
  firstLexicon->Add(DictEntryFactory::New("a", "X"));
  firstLexicon->Sort();
  LexiconPtr secondLexicon(new Lexicon);
  secondLexicon->Add(DictEntryFactory::New("ab", "Y"));
  secondLexicon->Sort();

  DictPtr firstDict(new TextDict(firstLexicon));
  DictPtr secondDict(new TextDict(secondLexicon));
  DictPtr dictGroup(new DictGroup(std::list<DictPtr>{firstDict, secondDict}));
  Conversion groupConversion(dictGroup);

  EXPECT_EQ("Xbc", groupConversion.Convert("abc"));
}

TEST_F(ConversionTest, PreserveNestedDictGroupPriority) {
  LexiconPtr firstLexicon(new Lexicon);
  firstLexicon->Add(DictEntryFactory::New("a", "X"));
  firstLexicon->Sort();
  LexiconPtr secondLexicon(new Lexicon);
  secondLexicon->Add(DictEntryFactory::New("ab", "Y"));
  secondLexicon->Sort();
  LexiconPtr thirdLexicon(new Lexicon);
  thirdLexicon->Add(DictEntryFactory::New("abc", "Z"));
  thirdLexicon->Sort();

  DictPtr firstDict(new TextDict(firstLexicon));
  DictPtr secondDict(new TextDict(secondLexicon));
  DictPtr thirdDict(new TextDict(thirdLexicon));
  DictPtr nestedGroup(new DictGroup(std::list<DictPtr>{firstDict, secondDict}));
  DictPtr dictGroup(new DictGroup(std::list<DictPtr>{nestedGroup, thirdDict}));
  Conversion groupConversion(dictGroup);

  EXPECT_EQ("Xbc", groupConversion.Convert("abc"));
}

TEST_F(ConversionTest, PreserveIdeographicDescriptionSequenceComponents) {
  LexiconPtr lexicon(new Lexicon);
  lexicon->Add(DictEntryFactory::New(utf8("钅"), utf8("釒")));
  lexicon->Add(DictEntryFactory::New(utf8("只"), utf8("隻")));
  lexicon->Sort();

  DictPtr componentDict(new TextDict(lexicon));
  Conversion componentConversion(componentDict);

  EXPECT_EQ(utf8("⿰钅只隻"), componentConversion.Convert(utf8("⿰钅只只")));
}

TEST_F(ConversionTest, PreserveComplexIdeographicDescriptionSequenceComponents) {
  LexiconPtr lexicon(new Lexicon);
  lexicon->Add(DictEntryFactory::New(utf8("長"), utf8("长")));
  lexicon->Add(DictEntryFactory::New(utf8("馬"), utf8("马")));
  lexicon->Add(DictEntryFactory::New(utf8("心"), utf8("芯")));
  lexicon->Sort();

  DictPtr componentDict(new TextDict(lexicon));
  Conversion componentConversion(componentDict);

  EXPECT_EQ(
      utf8("⿺⻍⿳穴⿲月⿱⿲幺言幺⿲長馬長刂心长"),
      componentConversion.Convert(
          utf8("⿺⻍⿳穴⿲月⿱⿲幺言幺⿲長馬長刂心長")));
}

TEST_F(ConversionTest, DoesNotPreserveIsolatedIdeographicDescriptionCharacters) {
  LexiconPtr lexicon(new Lexicon);
  lexicon->Add(DictEntryFactory::New(utf8("表"), utf8("錶")));
  lexicon->Add(DictEntryFactory::New(utf8("证"), utf8("證")));
  lexicon->Sort();

  DictPtr dict(new TextDict(lexicon));
  Conversion idcDescriptionConversion(dict);

  EXPECT_EQ(
      utf8("錶意文字描述字元包括這些字元：⿰⿱⿲⿳⿴⿵⿶⿷⿸⿹⿺⿻⿼⿽⿾⿿"),
      idcDescriptionConversion.Convert(
          utf8("表意文字描述字元包括這些字元：⿰⿱⿲⿳⿴⿵⿶⿷⿸⿹⿺⿻⿼⿽⿾⿿")));
  EXPECT_EQ(utf8("⿾证證"), idcDescriptionConversion.Convert(utf8("⿾证证")));
}

TEST_F(ConversionTest, DifferentialFuzzAgainstReferenceLoop) {
  // Random inputs mixing dictionary keys, unrelated CJK, ASCII, IDS
  // operators, and raw (possibly invalid or truncated) bytes. The skip-scan
  // path must produce exactly what the per-character reference loop
  // produces — same output or the same exception — on BOTH PrefixMatch
  // constructions: the table path (dict group) and the single-dictionary
  // fast path (MarisaDict answering prefix queries directly, whose skip
  // table is built from a trie walk instead of materialized lexicons).
  const MarisaDictPtr marisaDict = MarisaDict::NewFromDict(*dict);
  const Conversion marisaConversion(marisaDict);
  const PrefixMatch marisaReference(marisaDict);
  PrefixMatch groupReference(dict);

  struct FuzzTarget {
    const char* name;
    const Conversion* conversion;
    const PrefixMatch* reference;
  };
  const std::vector<FuzzTarget> targets = {
      {"table-path", conversion.get(), &groupReference},
      {"fast-path", &marisaConversion, &marisaReference},
  };

  const std::vector<std::string> fragments = {
      utf8("太后"),   utf8("头发"),  utf8("干燥"),  utf8("鼠标"),
      utf8("干"),     utf8("后"),    utf8("发"),    utf8("里"),
      utf8("天地"),   utf8("、。"), utf8("⿰"),    utf8("⿳"),
      utf8("——"),   utf8("🎉"),    "ascii text ",  "12345",
  };
  for (const FuzzTarget& target : targets) {
    std::mt19937 rng(20260724);
    for (int iteration = 0; iteration < 300; iteration++) {
      std::string input;
      const int pieces = 1 + static_cast<int>(rng() % 40);
      for (int i = 0; i < pieces; i++) {
        const uint32_t choice = rng() % 10;
        if (choice < 7) {
          input += fragments[rng() % fragments.size()];
        } else if (choice < 9) {
          // Raw bytes, frequently invalid or truncated UTF-8.
          const int count = 1 + static_cast<int>(rng() % 4);
          for (int b = 0; b < count; b++) {
            input.push_back(static_cast<char>(rng() % 256));
          }
        } else if (!input.empty()) {
          // Truncate the tail mid-character.
          input.resize(input.size() - 1);
        }
      }
      bool referenceThrew = false;
      bool convertThrew = false;
      std::string expected, actual;
      try {
        expected = ReferenceConvert(*target.reference, input);
      } catch (const InvalidUTF8&) {
        referenceThrew = true;
      }
      try {
        actual = target.conversion->Convert(input);
      } catch (const InvalidUTF8&) {
        convertThrew = true;
      }
      ASSERT_EQ(referenceThrew, convertThrew)
          << target.name << " iteration " << iteration;
      if (!referenceThrew) {
        ASSERT_EQ(expected, actual)
            << target.name << " iteration " << iteration;
      }
    }
  }
}

TEST_F(ConversionTest, TruncatedUtf8Sequence) {
  // This test specifically triggers the information disclosure vulnerability
  // in the old code. The bug occurs when a string ends with an incomplete
  // UTF-8 sequence.
  //
  // Background: UTF8Util::NextCharLength() examines only the first byte to
  // determine the expected character length (1-6 bytes), but doesn't verify
  // that enough bytes actually remain before the null terminator.
  //
  // Trigger condition: When the expected UTF-8 character length exceeds
  // the actual remaining bytes before null, the old code would:
  // 1. Call FromSubstr with a length crossing the null terminator
  // 2. Advance pstr beyond the null terminator
  // 3. Continue reading heap memory on next iteration
  // 4. Output leaked heap data to conversion result (INFORMATION DISCLOSURE)

  // Construct a string ending with a truncated 3-byte UTF-8 sequence:
  // - Normal text: "干" (valid 3-byte UTF-8: 0xE5 0xB9 0xB2)
  // - Followed by: 0xE5 0xB9 (incomplete 3-byte sequence - missing last byte)
  std::string malformed;
  malformed += utf8("干");   // Valid character
  malformed += '\xE5';       // Start of 3-byte UTF-8 (NextCharLength returns 3)
  malformed += '\xB9';       // Second byte
  // Missing third byte - only 2 bytes remain but NextCharLength expects 3
  // Old code would jump over null, read heap memory, and leak it in output

  // The fixed code should handle this gracefully without information disclosure
  EXPECT_NO_THROW({
    const std::string converted = conversion->Convert(malformed);
    // Should convert "干" to "幹" (first candidate in dict) and preserve incomplete sequence
    std::string expected;
    expected += utf8("幹");  // Converted from "干" (dict has ["幹", "乾", "干"])
    expected += '\xE5';      // Incomplete sequence preserved as-is
    expected += '\xB9';
    EXPECT_EQ(expected, converted);
    // Should NOT contain garbage heap data beyond the input
    // (ASan would catch any out-of-bounds reads during conversion)
  });
}

} // namespace opencc
