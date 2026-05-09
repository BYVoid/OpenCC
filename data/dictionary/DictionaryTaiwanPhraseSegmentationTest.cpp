/*
 * Open Chinese Convert
 *
 * Validate that Taiwan phrase keys are not interrupted by Simplified-to-
 * Traditional phrase values during s2twp conversion.
 */

#include "gtest/gtest.h"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "src/Exception.hpp"
#include "src/Lexicon.hpp"
#include "src/UTF8Util.hpp"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

namespace opencc {
namespace {

static FILE* OpenFile(const std::string& path) {
#ifdef _MSC_VER
  return _wfopen(UTF8Util::GetPlatformString(path).c_str(), L"rb");
#else
  return fopen(UTF8Util::GetPlatformString(path).c_str(), "rb");
#endif
}

LexiconPtr LoadLexicon(const std::string& path) {
  FILE* fp = OpenFile(path);
  EXPECT_NE(fp, nullptr) << path;
  if (fp == nullptr) {
    return LexiconPtr();
  }
  return Lexicon::ParseLexiconFromFile(fp);
}

bool HasAtLeastTwoCharacters(const std::string& text) {
  return UTF8Util::Length(text.c_str()) >= 2;
}

struct STPhraseValue {
  size_t lineNumber;
  std::string key;
  std::string value;
};

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

std::vector<STPhraseValue> LoadSTPhraseValues(const std::string& path) {
  std::ifstream stream(path);
  if (!stream.is_open()) {
    ADD_FAILURE() << path;
    return std::vector<STPhraseValue>();
  }

  std::vector<STPhraseValue> values;
  std::unordered_set<std::string> seenValues;
  std::string line;
  size_t lineNumber = 0;
  while (std::getline(stream, line)) {
    ++lineNumber;
    if (line.empty() || line[0] == '#') {
      continue;
    }

    const size_t tab = line.find('\t');
    if (tab == std::string::npos) {
      continue;
    }

    const std::string key = line.substr(0, tab);
    const std::string valueText = line.substr(tab + 1);
    for (const std::string& value : SplitValues(valueText)) {
      if (seenValues.insert(value).second) {
        values.push_back(STPhraseValue{lineNumber, key, value});
      }
    }
  }
  return values;
}

bool IsCoveredBySTPhraseValue(const std::string& key,
                              const std::vector<STPhraseValue>& stValues) {
  for (const STPhraseValue& stValue : stValues) {
    if (stValue.value.find(key) != std::string::npos) {
      return true;
    }
  }
  return false;
}

struct FragmentRisk {
  std::string category;
  size_t lineNumber;
  std::string key;
  std::string value;
};

bool FindSTPhraseValueFragment(const std::string& key,
                               const std::vector<STPhraseValue>& stValues,
                               FragmentRisk* risk) {
  for (const STPhraseValue& stValue : stValues) {
    const std::string& value = stValue.value;
    if (value.size() >= key.size() || !HasAtLeastTwoCharacters(value)) {
      continue;
    }

    const size_t pos = key.find(value);
    if (pos == std::string::npos) {
      continue;
    }

    if (pos == 0) {
      risk->category = "prefix";
    } else if (pos + value.size() == key.size()) {
      risk->category = "suffix";
    } else {
      risk->category = "middle";
    }
    risk->lineNumber = stValue.lineNumber;
    risk->key = stValue.key;
    risk->value = value;
    return true;
  }
  return false;
}

TEST(DictionaryTaiwanPhraseSegmentationTest,
     TWPhrasesKeysAreCoveredBySTPhraseValues) {
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest());
  ASSERT_NE(nullptr, runfiles);

  const std::string stPhrasesFile =
      runfiles->Rlocation("_main/data/dictionary/STPhrases.txt");
  const std::string twPhrasesFile =
      runfiles->Rlocation("_main/data/dictionary/TWPhrases.txt");

  try {
    LexiconPtr twPhrases = LoadLexicon(twPhrasesFile);
    ASSERT_NE(twPhrases, nullptr);

    std::vector<STPhraseValue> stValues = LoadSTPhraseValues(stPhrasesFile);
    ASSERT_FALSE(stValues.empty());

    std::ostringstream missing;
    size_t missingCount = 0;
    for (size_t i = 0; i < twPhrases->Length(); ++i) {
      const std::string key = twPhrases->At(i)->Key();
      if (IsCoveredBySTPhraseValue(key, stValues)) {
        continue;
      }

      FragmentRisk risk;
      if (!FindSTPhraseValueFragment(key, stValues, &risk)) {
        continue;
      }

      ++missingCount;
      missing << "TWPhrases key \"" << key
              << "\" is not covered by any STPhrases value.\n"
              << "  Conflicting STPhrases record: STPhrases.txt:"
              << risk.lineNumber << " \"" << risk.key
              << "\" -> \"" << risk.value << "\"\n"
              << "  The existing value appears as a " << risk.category
              << " fragment of the Taiwan phrase key.\n";
    }

    EXPECT_EQ(0U, missingCount)
        << "Potential missing STPhrases entries for s2twp segmentation:\n"
        << missing.str();
  } catch (const Exception& ex) {
    FAIL() << "Exception: " << ex.what();
  } catch (const std::exception& ex) {
    FAIL() << "std::exception: " << ex.what();
  } catch (...) {
    FAIL() << "Unknown exception thrown during Taiwan phrase segmentation "
              "check.";
  }
}

} // namespace
} // namespace opencc
