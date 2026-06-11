/*
 * Open Chinese Convert
 *
 * Validate that phrase-level character substitutions remain declared in the
 * corresponding character dictionary.
 */

#include "gtest/gtest.h"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/UTF8Util.hpp"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

namespace opencc {
namespace {

struct Entry {
  size_t lineNumber;
  std::string key;
  std::vector<std::string> values;
};

struct DictionaryPair {
  std::string phraseFile;
  std::string characterFile;
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

std::vector<std::string> SplitCharacters(const std::string& text) {
  std::vector<std::string> characters;
  const char* cursor = text.c_str();
  while (*cursor != '\0') {
    const size_t length = UTF8Util::NextCharLength(cursor);
    characters.push_back(UTF8Util::FromSubstr(cursor, length));
    cursor += length;
  }
  return characters;
}

bool IsDictionaryEntry(const std::string& line) {
  return !line.empty() && line[0] != '#';
}

std::vector<Entry> LoadEntries(const std::string& path) {
  std::ifstream stream(path);
  EXPECT_TRUE(stream.is_open()) << path;

  std::vector<Entry> entries;
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
    entries.push_back(Entry{lineNumber, line.substr(0, tab),
                            SplitValues(line.substr(tab + 1))});
  }
  return entries;
}

std::unordered_map<std::string, std::unordered_set<std::string>>
LoadCharacterCandidates(const std::string& path) {
  std::unordered_map<std::string, std::unordered_set<std::string>> candidates;
  for (const Entry& entry : LoadEntries(path)) {
    if (UTF8Util::Length(entry.key.c_str()) != 1) {
      continue;
    }
    for (const std::string& value : entry.values) {
      if (UTF8Util::Length(value.c_str()) == 1) {
        candidates[entry.key].insert(value);
      }
    }
  }
  return candidates;
}

void ExpectPhraseCharacterSubstitutionsDeclared(
    const Runfiles& runfiles, const DictionaryPair& dictionaryPair) {
  const std::string dictionaryDir = "_main/data/dictionary/";
  const std::string phrasePath =
      runfiles.Rlocation(dictionaryDir + dictionaryPair.phraseFile);
  const std::string characterPath =
      runfiles.Rlocation(dictionaryDir + dictionaryPair.characterFile);

  const std::vector<Entry> phraseEntries = LoadEntries(phrasePath);
  const std::unordered_map<std::string, std::unordered_set<std::string>>
      characterCandidates = LoadCharacterCandidates(characterPath);

  std::ostringstream failures;
  size_t failureCount = 0;
  for (const Entry& entry : phraseEntries) {
    const std::vector<std::string> keyCharacters = SplitCharacters(entry.key);
    for (const std::string& value : entry.values) {
      const std::vector<std::string> valueCharacters = SplitCharacters(value);
      if (keyCharacters.size() != valueCharacters.size()) {
        continue;
      }

      for (size_t i = 0; i < keyCharacters.size(); ++i) {
        const std::string& source = keyCharacters[i];
        const std::string& target = valueCharacters[i];
        if (source == target) {
          continue;
        }

        const auto candidates = characterCandidates.find(source);
        if (candidates != characterCandidates.end() &&
            candidates->second.count(target) != 0) {
          continue;
        }

        ++failureCount;
        failures << dictionaryPair.phraseFile << ":" << entry.lineNumber
                 << " maps \"" << entry.key << "\" -> \"" << value
                 << "\", including character substitution \"" << source
                 << "\" -> \"" << target << "\", but "
                 << dictionaryPair.characterFile
                 << " does not list that target as a candidate.\n";
      }
    }
  }

  EXPECT_EQ(0U, failureCount)
      << "Phrase-level character substitutions must also be declared in the "
         "corresponding character dictionary. If a direct character conversion "
         "is not generally desirable, keep it as a non-default candidate such "
         "as A<TAB>A B instead of deleting it outright:\n"
      << failures.str();
}

TEST(DictionaryPhraseCharacterDependencyTest,
     PhraseSubstitutionsRemainDeclaredInCharacterDictionaries) {
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest());
  ASSERT_NE(nullptr, runfiles);

  const std::vector<DictionaryPair> dictionaryPairs = {
      {"STPhrases.txt", "STCharacters.txt"},
      {"TSPhrases.txt", "TSCharacters.txt"},
      {"HKVariantsPhrases.txt", "HKVariants.txt"},
      {"HKVariantsRevPhrases.txt", "HKVariantsRev.txt"},
      {"TWVariantsPhrases.txt", "TWVariants.txt"},
      {"TWVariantsRevPhrases.txt", "TWVariantsRev.txt"},
  };
  for (const DictionaryPair& dictionaryPair : dictionaryPairs) {
    ExpectPhraseCharacterSubstitutionsDeclared(*runfiles, dictionaryPair);
  }
}

} // namespace
} // namespace opencc
