/*
 * Open Chinese Convert
 *
 * Copyright 2010-2026 Carbo Kuo and contributors
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

#include <cctype>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>

#include "DictConverter.hpp"
#include "Dict.hpp"
#include "DictEntry.hpp"
#include "Exception.hpp"
#include "Lexicon.hpp"
#include "MarisaDict.hpp"
#include "TextDict.hpp"
#if defined(_WIN32) || defined(_WIN64)
#include "WinUtil.hpp"
#endif

#include "DartsDict.hpp"

using namespace opencc;

namespace {

std::vector<std::string> SplitWhitespace(const std::string& line) {
  std::vector<std::string> tokens;
  std::istringstream iss(line);
  std::string token;
  while (iss >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

bool IsCommentOrEmpty(const std::string& line) {
  if (line.empty()) {
    return true;
  }
  for (size_t i = 0; i < line.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(line[i]);
    if (!std::isspace(ch)) {
      return false;
    }
  }
  return true;
}

std::vector<std::string>
ParseCppJiebaBaseDictValues(const std::vector<std::string>& tokens,
                            size_t lineNumber) {
  if (tokens.size() != 3) {
    throw Exception("Invalid cppjieba_utf8 base dict line: " +
                    std::to_string(lineNumber));
  }
  return std::vector<std::string>{tokens[1], tokens[2], "base"};
}

std::vector<std::string>
ParseCppJiebaUserDictValues(const std::vector<std::string>& tokens,
                            size_t lineNumber) {
  if (tokens.empty() || tokens.size() > 3) {
    throw Exception("Invalid cppjieba_utf8 user dict line: " +
                    std::to_string(lineNumber));
  }
  if (tokens.size() == 1) {
    return std::vector<std::string>{"", "", "user_default"};
  }
  if (tokens.size() == 2) {
    return std::vector<std::string>{"", tokens[1], "user_default"};
  }
  return std::vector<std::string>{tokens[1], tokens[2], "user_freq"};
}

class LexiconDict : public Dict {
public:
  explicit LexiconDict(std::vector<std::unique_ptr<DictEntry>> entries)
      : lexicon_(new Lexicon(std::move(entries))), maxLength_(0) {
    for (size_t i = 0; i < lexicon_->Length(); ++i) {
      const DictEntry* entry = lexicon_->At(i);
      if (entry->KeyLength() > maxLength_) {
        maxLength_ = entry->KeyLength();
      }
    }
  }

  Optional<const DictEntry*> Match(const char* word, size_t len) const override {
    const std::string needle(word, len);
    for (size_t i = 0; i < lexicon_->Length(); ++i) {
      const DictEntry* entry = lexicon_->At(i);
      if (entry->Key() == needle) {
        return Optional<const DictEntry*>(entry);
      }
    }
    return Optional<const DictEntry*>::Null();
  }

  Optional<const DictEntry*> MatchPrefix(const char* word,
                                         size_t len) const override {
    const size_t cappedLen = len < maxLength_ ? len : maxLength_;
    const DictEntry* best = nullptr;
    size_t bestLength = 0;
    for (size_t i = 0; i < lexicon_->Length(); ++i) {
      const DictEntry* entry = lexicon_->At(i);
      const size_t entryLength = entry->KeyLength();
      if (entryLength > cappedLen || entryLength <= bestLength) {
        continue;
      }
      if (std::string(word, entryLength) == entry->Key()) {
        best = entry;
        bestLength = entryLength;
      }
    }
    if (best == nullptr) {
      return Optional<const DictEntry*>::Null();
    }
    return Optional<const DictEntry*>(best);
  }

  std::vector<const DictEntry*> MatchAllPrefixes(const char* word,
                                                 size_t len) const override {
    const size_t cappedLen = len < maxLength_ ? len : maxLength_;
    std::vector<const DictEntry*> matches;
    for (size_t i = 0; i < lexicon_->Length(); ++i) {
      const DictEntry* entry = lexicon_->At(i);
      const size_t entryLength = entry->KeyLength();
      if (entryLength > cappedLen) {
        continue;
      }
      if (std::string(word, entryLength) == entry->Key()) {
        matches.push_back(entry);
      }
    }
    return matches;
  }

  size_t KeyMaxLength() const override { return maxLength_; }

  LexiconPtr GetLexicon() const override { return lexicon_; }

private:
  LexiconPtr lexicon_;
  size_t maxLength_;
};

void LoadCppJiebaBaseDict(
    const std::string& path,
    std::map<std::string, std::vector<std::string>>& entries) {
  std::ifstream ifs(path.c_str());
  if (!ifs.is_open()) {
    throw FileNotFound(path);
  }

  std::string line;
  size_t lineNumber = 0;
  while (std::getline(ifs, line)) {
    ++lineNumber;
    if (IsCommentOrEmpty(line)) {
      continue;
    }
    const std::vector<std::string> tokens = SplitWhitespace(line);
    entries[tokens[0]] = ParseCppJiebaBaseDictValues(tokens, lineNumber);
  }
}

void LoadCppJiebaUserDict(
    const std::string& path,
    std::map<std::string, std::vector<std::string>>& entries) {
  std::ifstream ifs(path.c_str());
  if (!ifs.is_open()) {
    throw FileNotFound(path);
  }

  std::string line;
  size_t lineNumber = 0;
  while (std::getline(ifs, line)) {
    ++lineNumber;
    if (IsCommentOrEmpty(line)) {
      continue;
    }
    const std::vector<std::string> tokens = SplitWhitespace(line);
    if (tokens.empty()) {
      continue;
    }
    entries[tokens[0]] = ParseCppJiebaUserDictValues(tokens, lineNumber);
  }
}

DictPtr LoadCppJiebaUtf8Dictionaries(
    const std::vector<std::string>& inputFileNames) {
  if (inputFileNames.empty()) {
    throw Exception("cppjieba_utf8 requires at least one input dictionary.");
  }

  std::map<std::string, std::vector<std::string>> entries;
  LoadCppJiebaBaseDict(inputFileNames.front(), entries);
  for (size_t i = 1; i < inputFileNames.size(); ++i) {
    LoadCppJiebaUserDict(inputFileNames[i], entries);
  }

  std::vector<std::unique_ptr<DictEntry>> lexiconEntries;
  lexiconEntries.reserve(entries.size());
  for (const auto& entry : entries) {
    lexiconEntries.push_back(std::unique_ptr<DictEntry>(
        DictEntryFactory::New(entry.first, entry.second)));
  }

  return DictPtr(new LexiconDict(std::move(lexiconEntries)));
}

} // namespace

DictPtr LoadDictionary(const std::string& format,
                       const std::string& inputFileName) {
  if (format == "text") {
    FILE* fp = nullptr;
#if defined(_WIN32) || defined(_WIN64)
    fp = _wfopen(internal::WideFromUtf8(inputFileName).c_str(), L"r");
#else
    fp = fopen(inputFileName.c_str(), "r");
#endif
    if (!fp) {
      throw FileNotFound(inputFileName);
    }
    DictPtr dict = TextDict::NewFromFile(fp);
    fclose(fp);
    return dict;
  } else if (format == "ocd") {
    return SerializableDict::NewFromFile<DartsDict>(inputFileName);
  } else if (format == "ocd2") {
    return SerializableDict::NewFromFile<MarisaDict>(inputFileName);
  }
  fprintf(stderr, "Unknown dictionary format: %s\n", format.c_str());
  exit(2);
  return nullptr;
}

SerializableDictPtr ConvertDict(const std::string& format, const DictPtr dict) {
  if (format == "text") {
    return TextDict::NewFromDict(*dict.get());
  } else if (format == "ocd") {
    return DartsDict::NewFromDict(*dict.get());
  } else if (format == "ocd2") {
    return MarisaDict::NewFromDict(*dict.get());
  }
  fprintf(stderr, "Unknown dictionary format: %s\n", format.c_str());
  exit(2);
  return nullptr;
}

namespace opencc {
void ConvertDictionary(const std::string& inputFileName,
                       const std::string& outputFileName,
                       const std::string& formatFrom,
                       const std::string& formatTo) {
  ConvertDictionary(std::vector<std::string>{inputFileName}, outputFileName,
                    formatFrom, formatTo);
}

void ConvertDictionary(const std::vector<std::string>& inputFileNames,
                       const std::string& outputFileName,
                       const std::string& formatFrom,
                       const std::string& formatTo) {
  DictPtr dictFrom;
  if (formatFrom == "cppjieba_utf8") {
    dictFrom = LoadCppJiebaUtf8Dictionaries(inputFileNames);
  } else {
    if (inputFileNames.size() != 1) {
      throw Exception("Dictionary format '" + formatFrom +
                      "' requires exactly one input dictionary.");
    }
    dictFrom = LoadDictionary(formatFrom, inputFileNames.front());
  }
  SerializableDictPtr dictTo = ConvertDict(formatTo, dictFrom);
  dictTo->SerializeToFile(outputFileName);
}
} // namespace opencc
