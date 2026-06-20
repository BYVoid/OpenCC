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
  std::string source;
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

std::vector<STPhraseValue> LoadSTPhraseValues(const std::string& path,
                                              const std::string& source) {
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
        values.push_back(STPhraseValue{source, lineNumber, key, value});
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
  std::string source;
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
    risk->source = stValue.source;
    risk->lineNumber = stValue.lineNumber;
    risk->key = stValue.key;
    risk->value = value;
    return true;
  }
  return false;
}

void ExpectProperSegmentationCoverage(const std::string& phrasesName) {
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest());
  ASSERT_NE(nullptr, runfiles);

  const std::string stPhrasesFile =
      runfiles->Rlocation("_main/data/dictionary/STPhrases.txt");
  const std::string generatedSTPhrasesFile = runfiles->Rlocation(
      "_main/data/dictionary/STPhrases_GeneratedFromRegionalPhrases.txt");
  const std::string phrasesFile =
      runfiles->Rlocation("_main/data/dictionary/" + phrasesName + ".txt");

  try {
    LexiconPtr phrases = LoadLexicon(phrasesFile);
    ASSERT_NE(phrases, nullptr);

    std::vector<STPhraseValue> stValues =
        LoadSTPhraseValues(stPhrasesFile, "STPhrases.txt");
    std::vector<STPhraseValue> generatedSTValues = LoadSTPhraseValues(
        generatedSTPhrasesFile, "STPhrases_GeneratedFromRegionalPhrases.txt");
    stValues.insert(stValues.end(), generatedSTValues.begin(),
                    generatedSTValues.end());
    ASSERT_FALSE(stValues.empty());

    std::ostringstream missing;
    size_t missingCount = 0;
    for (size_t i = 0; i < phrases->Length(); ++i) {
      const std::string key = phrases->At(i)->Key();
      if (IsCoveredBySTPhraseValue(key, stValues)) {
        continue;
      }

      FragmentRisk risk;
      if (!FindSTPhraseValueFragment(key, stValues, &risk)) {
        continue;
      }

      ++missingCount;
      missing << phrasesName << " key \"" << key
              << "\" is not covered by any ST phrase value.\n"
              << "  Conflicting ST phrase record: " << risk.source << ":"
              << risk.lineNumber << " \"" << risk.key
              << "\" -> \"" << risk.value << "\"\n"
              << "  The existing value appears as a " << risk.category
              << " fragment of the " << phrasesName << " key.\n";
    }

    EXPECT_EQ(0U, missingCount)
        << "Potential missing ST phrase entries for " << phrasesName << ":\n"
        << missing.str();
  } catch (const Exception& ex) {
    FAIL() << "Exception: " << ex.what();
  } catch (const std::exception& ex) {
    FAIL() << "std::exception: " << ex.what();
  } catch (...) {
    FAIL() << "Unknown exception thrown during " << phrasesName << " segmentation "
              "check.";
  }
}

void ExpectGeneratedSTPhrasesFromRegionalPhrasesAreValid() {
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest());
  ASSERT_NE(nullptr, runfiles);

  const std::string hkPhrasesFile =
      runfiles->Rlocation("_main/data/dictionary/HKPhrases.txt");
  const std::string twPhrasesFile =
      runfiles->Rlocation("_main/data/dictionary/TWPhrases.txt");
  const std::string generatedName = "STPhrases_GeneratedFromRegionalPhrases";
  const std::string generatedSTPhrasesFile = runfiles->Rlocation(
      "_main/data/dictionary/" + generatedName + ".txt");

  try {
    LexiconPtr hkPhrases = LoadLexicon(hkPhrasesFile);
    ASSERT_NE(hkPhrases, nullptr);
    LexiconPtr twPhrases = LoadLexicon(twPhrasesFile);
    ASSERT_NE(twPhrases, nullptr);
    LexiconPtr generatedSTPhrases = LoadLexicon(generatedSTPhrasesFile);
    ASSERT_NE(generatedSTPhrases, nullptr);

    std::unordered_set<std::string> phraseKeys;
    for (size_t i = 0; i < hkPhrases->Length(); ++i) {
      phraseKeys.insert(hkPhrases->At(i)->Key());
    }
    for (size_t i = 0; i < twPhrases->Length(); ++i) {
      phraseKeys.insert(twPhrases->At(i)->Key());
    }

    for (size_t i = 0; i < generatedSTPhrases->Length(); ++i) {
      const std::string key = generatedSTPhrases->At(i)->Key();
      EXPECT_GE(UTF8Util::Length(key.c_str()), 3U)
          << "Short generated ST phrase key may split longer words: " << key;

      const std::vector<std::string> values = generatedSTPhrases->At(i)->Values();
      for (const std::string& value : values) {
        EXPECT_NE(phraseKeys.count(value), 0U)
            << generatedName << " value is not a regional phrase key: "
            << value;
      }
    }
  } catch (const Exception& ex) {
    FAIL() << "Exception: " << ex.what();
  } catch (const std::exception& ex) {
    FAIL() << "std::exception: " << ex.what();
  } catch (...) {
    FAIL() << "Unknown exception thrown during " << generatedName
           << " coverage check.";
  }
}

TEST(DictionaryPhraseSegmentationTest,
     HKPhrasesKeysAreCoveredBySTPhraseValues) {
  ExpectProperSegmentationCoverage("HKPhrases");
}

TEST(DictionaryPhraseSegmentationTest,
     GeneratedSTPhrasesFromRegionalPhrasesAreValid) {
  ExpectGeneratedSTPhrasesFromRegionalPhrasesAreValid();
}

} // namespace
} // namespace opencc
