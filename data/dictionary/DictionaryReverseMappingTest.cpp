/*
 * Open Chinese Convert
 *
 * Validate generated reverse phrase dictionaries.
 */

#include "gtest/gtest.h"

#include <unordered_map>
#include <unordered_set>

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

TEST(DictionaryReverseMappingTest, TWPhrasesReverseMapping) {
  std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest());
  ASSERT_NE(nullptr, runfiles);

  const std::string twPhrasesFile =
      runfiles->Rlocation("_main/data/dictionary/TWPhrases.txt");
  const std::string twPhrasesRevFile =
      runfiles->Rlocation("_main/data/dictionary/TWPhrasesRev.txt");

  auto loadLexicon = [](const std::string& path) -> LexiconPtr {
    FILE* fp = OpenFile(path);
    EXPECT_NE(fp, nullptr) << path;
    if (fp == nullptr) {
      return LexiconPtr();
    }
    return Lexicon::ParseLexiconFromFile(fp);
  };

  auto buildMap = [](const LexiconPtr& lexicon)
      -> std::unordered_map<std::string, std::unordered_set<std::string>> {
    std::unordered_map<std::string, std::unordered_set<std::string>> map;
    if (!lexicon) {
      return map;
    }
    for (size_t i = 0; i < lexicon->Length(); ++i) {
      const DictEntry* entry = lexicon->At(i);
      auto& values = map[entry->Key()];
      for (const auto& value : entry->Values()) {
        values.insert(value);
      }
    }
    return map;
  };

  try {
    LexiconPtr twPhrases = loadLexicon(twPhrasesFile);
    LexiconPtr twPhrasesRev = loadLexicon(twPhrasesRevFile);
    ASSERT_NE(twPhrases, nullptr);
    ASSERT_NE(twPhrasesRev, nullptr);

    auto twMap = buildMap(twPhrases);
    auto twRevMap = buildMap(twPhrasesRev);

    for (const auto& entry : twMap) {
      const std::string& key = entry.first;
      for (const auto& value : entry.second) {
        auto it = twRevMap.find(value);
        EXPECT_TRUE(it != twRevMap.end() && it->second.count(key) > 0)
            << "Missing reverse mapping: " << key << " -> " << value;
      }
    }

    for (const auto& entry : twRevMap) {
      const std::string& key = entry.first;
      for (const auto& value : entry.second) {
        auto it = twMap.find(value);
        EXPECT_TRUE(it != twMap.end() && it->second.count(key) > 0)
            << "Missing reverse mapping: " << key << " -> " << value;
      }
    }
  } catch (const Exception& ex) {
    FAIL() << "Exception: " << ex.what();
  } catch (const std::exception& ex) {
    FAIL() << "std::exception: " << ex.what();
  } catch (...) {
    FAIL() << "Unknown exception thrown during reverse mapping check.";
  }
}

} // namespace
} // namespace opencc
