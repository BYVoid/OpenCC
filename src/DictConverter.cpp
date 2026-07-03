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
#include <sstream>

#include "DictConverter.hpp"
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

std::vector<std::string> ParseCppJiebaBaseValues(
    const std::vector<std::string>& tokens, size_t lineNumber) {
  if (tokens.size() != 3) {
    throw InvalidFormat("Invalid cppjieba_utf8 base dict line: " +
                        std::to_string(lineNumber));
  }
  return std::vector<std::string>{tokens[1], tokens[2], "base"};
}

std::vector<std::string> ParseCppJiebaUserValues(
    const std::vector<std::string>& tokens, size_t lineNumber) {
  if (tokens.empty() || tokens.size() > 3) {
    throw InvalidFormat("Invalid cppjieba_utf8 user dict line: " +
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

void LoadCppJiebaUtf8Dict(
    const std::string& path, bool isBaseDict,
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
    entries[tokens[0]] =
        isBaseDict ? ParseCppJiebaBaseValues(tokens, lineNumber)
                   : ParseCppJiebaUserValues(tokens, lineNumber);
  }
}

DictPtr LoadCppJiebaUtf8Dictionaries(
    const std::vector<std::string>& inputFileNames) {
  if (inputFileNames.empty()) {
    throw InvalidFormat("cppjieba_utf8 requires at least one input dictionary.");
  }

  std::map<std::string, std::vector<std::string>> entries;
  LoadCppJiebaUtf8Dict(inputFileNames.front(), true, entries);
  for (size_t i = 1; i < inputFileNames.size(); ++i) {
    LoadCppJiebaUtf8Dict(inputFileNames[i], false, entries);
  }

  LexiconPtr lexicon(new Lexicon);
  for (const auto& entry : entries) {
    lexicon->Add(DictEntryFactory::New(entry.first, entry.second));
  }
  return TextDictPtr(new TextDict(lexicon));
}

} // namespace

DictPtr LoadDictionary(const std::string& format,
                       const std::string& inputFileName) {
  if (format == "text" || format == "text_space") {
    FILE* fp = nullptr;
#if defined(_WIN32) || defined(_WIN64)
    fp = _wfopen(internal::WideFromUtf8(inputFileName).c_str(), L"r");
#else
    fp = fopen(inputFileName.c_str(), "r");
#endif
    if (!fp) {
      throw FileNotFound(inputFileName);
    }
    DictPtr dict = format == "text" ? TextDict::NewFromFile(fp)
                                    : TextDict::NewFromFile(fp, ' ');
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
      throw InvalidFormat("Dictionary format '" + formatFrom +
                          "' requires exactly one input dictionary.");
    }
    dictFrom = LoadDictionary(formatFrom, inputFileNames.front());
  }
  SerializableDictPtr dictTo = ConvertDict(formatTo, dictFrom);
  dictTo->SerializeToFile(outputFileName);
}
} // namespace opencc
