/*
 * Open Chinese Convert
 *
 * Validate reverse variant phrase dictionary coverage.
 */

#include "gtest/gtest.h"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "src/UTF8Util.hpp"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

namespace opencc {
namespace {

std::vector<std::string> SplitValues(const std::string& values) {
  std::vector<std::string> split;
  size_t start = 0;
  while (start <= values.size()) {
    const size_t end = values.find(' ', start);
    split.push_back(values.substr(start, end - start));
    if (end == std::string::npos) {
      break;
    }
    start = end + 1;
  }
  return split;
}

bool IsDictionaryEntry(const std::string& line) {
  return !line.empty() && line[0] != '#';
}

std::vector<std::unordered_set<std::string>> LoadNonEquivalentVariantSets(
    const std::string& path) {
  std::ifstream stream(path);
  EXPECT_TRUE(stream.is_open()) << path;

  std::vector<std::unordered_set<std::string>> variantSets;
  std::string line;
  while (std::getline(stream, line)) {
    if (!IsDictionaryEntry(line)) {
      continue;
    }

    const size_t firstTab = line.find('\t');
    if (firstTab == std::string::npos) {
      continue;
    }
    const size_t secondTab = line.find('\t', firstTab + 1);
    const std::string variantsText =
        line.substr(firstTab + 1, secondTab - firstTab - 1);
    std::vector<std::string> variants = SplitValues(variantsText);
    if (variants.size() <= 1) {
      continue;
    }

    variantSets.emplace_back(variants.begin(), variants.end());
  }
  return variantSets;
}

bool IsInSameNonEquivalentSet(
    const std::vector<std::unordered_set<std::string>>& variantSets,
    const std::string& first, const std::string& second) {
  for (const auto& variantSet : variantSets) {
    if (variantSet.count(first) != 0 && variantSet.count(second) != 0) {
      return true;
    }
  }
  return false;
}

std::unordered_set<std::string> LoadReverseExceptionCharacters(
    const std::string& variantsPath, const std::string& schemePath) {
  const std::vector<std::unordered_set<std::string>> variantSets =
      LoadNonEquivalentVariantSets(schemePath);
  EXPECT_FALSE(variantSets.empty());

  std::ifstream stream(variantsPath);
  EXPECT_TRUE(stream.is_open()) << variantsPath;

  std::unordered_set<std::string> exceptionChars;
  std::string line;
  while (std::getline(stream, line)) {
    if (!IsDictionaryEntry(line)) {
      continue;
    }

    const size_t tab = line.find('\t');
    if (tab == std::string::npos) {
      ADD_FAILURE() << line;
      continue;
    }
    const std::string key = line.substr(0, tab);
    const std::string values = line.substr(tab + 1);
    for (const std::string& value : SplitValues(values)) {
      if (key == value) {
        continue;
      }
      if (IsInSameNonEquivalentSet(variantSets, key, value)) {
        exceptionChars.insert(value);
      }
    }
  }
  return exceptionChars;
}

bool ContainsAny(const std::string& text,
                 const std::unordered_set<std::string>& needles) {
  for (const std::string& needle : needles) {
    if (text.find(needle) != std::string::npos) {
      return true;
    }
  }
  return false;
}

std::unordered_set<std::string> LoadExpectedPhraseExceptions(
    const std::string& stPhrasesPath,
    const std::unordered_set<std::string>& exceptionChars) {
  std::ifstream stream(stPhrasesPath);
  EXPECT_TRUE(stream.is_open()) << stPhrasesPath;

  std::unordered_set<std::string> expected;
  std::string line;
  while (std::getline(stream, line)) {
    if (!IsDictionaryEntry(line)) {
      continue;
    }

    const size_t tab = line.find('\t');
    if (tab == std::string::npos) {
      ADD_FAILURE() << line;
      continue;
    }
    const std::string values = line.substr(tab + 1);
    for (const std::string& value : SplitValues(values)) {
      if (UTF8Util::Length(value.c_str()) >= 2 &&
          ContainsAny(value, exceptionChars)) {
        expected.insert(value);
      }
    }
  }
  return expected;
}

std::unordered_set<std::string> LoadIdentityPhraseKeys(
    const std::string& phrasesPath) {
  std::ifstream stream(phrasesPath);
  EXPECT_TRUE(stream.is_open()) << phrasesPath;

  std::unordered_set<std::string> identityKeys;
  std::string line;
  while (std::getline(stream, line)) {
    if (!IsDictionaryEntry(line)) {
      continue;
    }

    const size_t tab = line.find('\t');
    if (tab == std::string::npos) {
      ADD_FAILURE() << line;
      continue;
    }
    const std::string key = line.substr(0, tab);
    const std::string values = line.substr(tab + 1);
    for (const std::string& value : SplitValues(values)) {
      if (key == value) {
        identityKeys.insert(key);
        break;
      }
    }
  }
  return identityKeys;
}

void ExpectRevPhrasesComplete(const std::string& variantsName) {
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest());
  ASSERT_NE(nullptr, runfiles);

  const std::string variantsPath =
      runfiles->Rlocation("_main/data/dictionary/" + variantsName + ".txt");
  const std::string phrasesPath = runfiles->Rlocation(
      "_main/data/dictionary/" + variantsName + "RevPhrases.txt");
  const std::string schemePath =
      runfiles->Rlocation("_main/data/scheme/st_multi.txt");
  const std::string stPhrasesPath =
      runfiles->Rlocation("_main/data/dictionary/STPhrases.txt");

  const std::unordered_set<std::string> exceptionChars =
      LoadReverseExceptionCharacters(variantsPath, schemePath);
  ASSERT_FALSE(exceptionChars.empty()) << variantsName;

  const std::unordered_set<std::string> expected =
      LoadExpectedPhraseExceptions(stPhrasesPath, exceptionChars);
  const std::unordered_set<std::string> identityKeys =
      LoadIdentityPhraseKeys(phrasesPath);

  std::ostringstream missing;
  for (const std::string& phrase : expected) {
    if (identityKeys.count(phrase) == 0) {
      missing << phrase << "\n";
    }
  }

  EXPECT_TRUE(missing.str().empty())
      << variantsName << " missing reverse phrase identity exceptions:\n"
      << missing.str();
}

TEST(DictionaryVariantRevPhrasesTest, HKVariantsAreComplete) {
  ExpectRevPhrasesComplete("HKVariants");
}

TEST(DictionaryVariantRevPhrasesTest, TWVariantsAreComplete) {
  ExpectRevPhrasesComplete("TWVariants");
}

} // namespace
} // namespace opencc
