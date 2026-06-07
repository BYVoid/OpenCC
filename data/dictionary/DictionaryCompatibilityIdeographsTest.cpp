/*
 * Open Chinese Convert
 *
 * Validate that regular dictionaries do not contain Unicode CJK Compatibility
 * Ideographs.
 */

#include "gtest/gtest.h"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "src/UTF8Util.hpp"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

namespace opencc {
namespace {

#ifndef OPENCC_COMPATIBILITY_AUDIT_FILES
#error "OPENCC_COMPATIBILITY_AUDIT_FILES must be defined"
#endif

bool IsDictionaryEntry(const std::string& line) {
  return !line.empty() && line[0] != '#';
}

std::vector<std::string> SplitCommaSeparated(const std::string& text) {
  std::vector<std::string> parts;
  size_t start = 0;
  while (start <= text.size()) {
    const size_t end = text.find(',', start);
    const std::string part = text.substr(start, end - start);
    if (!part.empty()) {
      parts.push_back(part);
    }
    if (end == std::string::npos) {
      break;
    }
    start = end + 1;
  }
  return parts;
}

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

std::unordered_set<std::string> LoadCompatibilityIdeographs(
    const std::string& path) {
  std::ifstream stream(UTF8Util::GetPlatformString(path));
  EXPECT_TRUE(stream.is_open()) << path;

  std::unordered_set<std::string> compatibilityIdeographs;
  std::string line;
  size_t lineNumber = 0;
  while (std::getline(stream, line)) {
    ++lineNumber;
    if (!IsDictionaryEntry(line)) {
      continue;
    }
    const size_t tab = line.find('\t');
    if (tab == std::string::npos) {
      ADD_FAILURE() << path << ":" << lineNumber
                    << " is missing a tab separator.";
      continue;
    }
    compatibilityIdeographs.insert(line.substr(0, tab));
  }
  return compatibilityIdeographs;
}

void ExpectNoCompatibilityIdeographs(
    const std::string& dictionaryName, const std::string& path,
    const std::unordered_set<std::string>& compatibilityIdeographs) {
  std::ifstream stream(UTF8Util::GetPlatformString(path));
  ASSERT_TRUE(stream.is_open()) << path;

  std::string line;
  size_t lineNumber = 0;
  while (std::getline(stream, line)) {
    ++lineNumber;
    if (!IsDictionaryEntry(line)) {
      continue;
    }

    for (const char* cursor = line.c_str(); *cursor != '\0';) {
      const size_t length = UTF8Util::NextCharLength(cursor);
      const std::string character = UTF8Util::FromSubstr(cursor, length);
      if (compatibilityIdeographs.count(character) != 0) {
        const uint32_t codePoint = DecodeUtf8CodePoint(cursor, length);
        ADD_FAILURE() << dictionaryName << ":" << lineNumber
                      << " contains CJK Compatibility Ideograph "
                      << FormatCodePoint(codePoint) << ". Use the standard "
                      << "CJK unified ideograph form instead.";
      }
      cursor += length;
    }
  }
}

TEST(DictionaryCompatibilityIdeographsTest,
     RegularDictionariesDoNotContainCompatibilityIdeographs) {
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest());
  ASSERT_NE(nullptr, runfiles);

  const std::string dictionaryDir = "_main/data/dictionary/";
  const std::string compatibilityPath = runfiles->Rlocation(
      dictionaryDir + "CJK_Compatibility_Ideographs.txt");
  const std::unordered_set<std::string> compatibilityIdeographs =
      LoadCompatibilityIdeographs(compatibilityPath);
  ASSERT_EQ(1002, compatibilityIdeographs.size());

  for (const std::string& dictionaryName :
       SplitCommaSeparated(OPENCC_COMPATIBILITY_AUDIT_FILES)) {
    const std::string dictionaryPath =
        runfiles->Rlocation(dictionaryDir + dictionaryName);
    ExpectNoCompatibilityIdeographs(dictionaryName, dictionaryPath,
                                    compatibilityIdeographs);
  }
}

} // namespace
} // namespace opencc
