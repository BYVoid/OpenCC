/*
 * Open Chinese Convert
 *
 * Validate non-BMP characters in phrase and variant dictionaries.
 */

#include "gtest/gtest.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>

#include "src/UTF8Util.hpp"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

namespace opencc {
namespace {

#ifndef OPENCC_DICTIONARY_NON_BMP_TEST_FILE
#error "OPENCC_DICTIONARY_NON_BMP_TEST_FILE must be defined"
#endif

struct NonBmpException {
  const char* dictionary;
  const char* character;
};

constexpr std::array<NonBmpException, 7> kAllowedNonBmpCharacters = {{
    {"STPhrases", "\xF0\xA3\xA8\xBC"}, // U+23A3C
    {"STPhrases", "\xF0\xAA\xA2\xAE"}, // U+2A8AE
    {"STPhrases", "\xF0\xAB\x96\xAE"}, // U+2B5AE
    {"STPhrases", "\xF0\xAB\x97\xA7"}, // U+2B5E7
    {"STPhrases", "\xF0\xAB\x9B\xAD"}, // U+2B6ED
    {"STPhrases", "\xF0\xAC\xB4\x83"}, // U+2CD03
    {"TSPhrases", "\xF0\xAB\xAB\x87"}, // U+2BAC7
}};

uint32_t DecodeUtf8CodePoint(const char* str, size_t length) {
  const unsigned char* bytes = reinterpret_cast<const unsigned char*>(str);
  if (length == 1) {
    return bytes[0];
  }
  if (length == 2) {
    return ((bytes[0] & 0x1F) << 6) | (bytes[1] & 0x3F);
  }
  if (length == 3) {
    return ((bytes[0] & 0x0F) << 12) | ((bytes[1] & 0x3F) << 6) |
           (bytes[2] & 0x3F);
  }
  if (length == 4) {
    return ((bytes[0] & 0x07) << 18) | ((bytes[1] & 0x3F) << 12) |
           ((bytes[2] & 0x3F) << 6) | (bytes[3] & 0x3F);
  }
  return 0;
}

std::string FormatCodePoint(uint32_t codePoint) {
  char buffer[11];
  snprintf(buffer, sizeof(buffer), "U+%04X", codePoint);
  return buffer;
}

bool IsAllowedNonBmpCharacter(const std::string& dictionary,
                              const std::string& character) {
  for (const auto& exception : kAllowedNonBmpCharacters) {
    if (dictionary == exception.dictionary && character == exception.character) {
      return true;
    }
  }
  return false;
}

TEST(DictionaryNonBmpTest, OnlyAllowlistedCharactersOutsideCharacterDicts) {
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest());
  ASSERT_NE(nullptr, runfiles);

  std::string dictionaryName = OPENCC_DICTIONARY_NON_BMP_TEST_FILE;
  const std::string suffix = ".txt";
  ASSERT_TRUE(dictionaryName.size() >= suffix.size());
  ASSERT_EQ(suffix, dictionaryName.substr(dictionaryName.size() - suffix.size()));
  dictionaryName.erase(dictionaryName.size() - suffix.size());

  const std::string dictionaryFileName = runfiles->Rlocation(
      "_main/data/dictionary/" +
      std::string(OPENCC_DICTIONARY_NON_BMP_TEST_FILE));
  std::ifstream ifs(UTF8Util::GetPlatformString(dictionaryFileName));
  ASSERT_TRUE(ifs.is_open()) << dictionaryFileName;

  std::string line;
  size_t lineNumber = 0;
  while (std::getline(ifs, line)) {
    ++lineNumber;
    for (const char* cursor = line.c_str(); *cursor != '\0';) {
      const size_t length = UTF8Util::NextCharLength(cursor);
      const uint32_t codePoint = DecodeUtf8CodePoint(cursor, length);
      if (codePoint > 0xFFFF) {
        const std::string character(cursor, length);
        EXPECT_TRUE(IsAllowedNonBmpCharacter(dictionaryName, character))
            << dictionaryName << ":" << lineNumber
            << " contains non-BMP character " << FormatCodePoint(codePoint)
            << ". Add it to kAllowedNonBmpCharacters only if it is intentional.";
      }
      cursor += length;
    }
  }
}

} // namespace
} // namespace opencc
