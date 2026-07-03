#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(__has_include)
#if __has_include("src/Dict.hpp")
#include "src/Dict.hpp"
#include "src/DictEntry.hpp"
#include "src/Lexicon.hpp"
#include "src/MarisaDict.hpp"
#include "src/SerializableDict.hpp"
#elif __has_include("Dict.hpp")
#include "Dict.hpp"
#include "DictEntry.hpp"
#include "Lexicon.hpp"
#include "MarisaDict.hpp"
#include "SerializableDict.hpp"
#elif __has_include(<opencc/Dict.hpp>)
#include <opencc/Dict.hpp>
#include <opencc/DictEntry.hpp>
#include <opencc/Lexicon.hpp>
#include <opencc/MarisaDict.hpp>
#include <opencc/SerializableDict.hpp>
#else
#error "Unable to locate OpenCC headers"
#endif
#else
#include "Dict.hpp"
#include "DictEntry.hpp"
#include "Lexicon.hpp"
#include "MarisaDict.hpp"
#include "SerializableDict.hpp"
#endif

namespace {

using opencc::Dict;
using opencc::DictEntry;
using opencc::DictEntryFactory;
using opencc::Lexicon;
using opencc::LexiconPtr;
using opencc::MarisaDict;
using opencc::MarisaDictPtr;
using opencc::Optional;
using opencc::SerializableDict;

void PrintUsage(std::ostream& os) {
  os << "Usage: opencc_jieba_dict_build_tool -i <dict> [-i <dict> ...] "
        "-o <output.ocd2>"
     << std::endl
     << "  -i <dict>         Input dictionary file. The first -i is parsed as"
     << std::endl
     << "                    the base jieba dict; later -i files are parsed as"
     << std::endl
     << "                    user dict overlays." << std::endl
     << "  -o <output.ocd2>  Output OpenCC marisa dictionary file."
     << std::endl;
}

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
ParseBaseDictValues(const std::vector<std::string>& tokens, size_t lineNumber) {
  if (tokens.size() != 3) {
    throw std::runtime_error("invalid base dict line " +
                             std::to_string(lineNumber));
  }
  return std::vector<std::string>{tokens[1], tokens[2], "base"};
}

std::vector<std::string>
ParseUserDictValues(const std::vector<std::string>& tokens, size_t lineNumber) {
  if (tokens.empty() || tokens.size() > 3) {
    throw std::runtime_error("invalid user dict line " +
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

void LoadBaseDict(const std::string& path,
                  std::map<std::string, std::vector<std::string>>& entries) {
  std::ifstream ifs(path.c_str());
  if (!ifs.is_open()) {
    throw std::runtime_error("failed to open base dict: " + path);
  }

  std::string line;
  size_t lineNumber = 0;
  while (std::getline(ifs, line)) {
    ++lineNumber;
    if (IsCommentOrEmpty(line)) {
      continue;
    }
    const std::vector<std::string> tokens = SplitWhitespace(line);
    entries[tokens[0]] = ParseBaseDictValues(tokens, lineNumber);
  }
}

void LoadUserDict(const std::string& path,
                  std::map<std::string, std::vector<std::string>>& entries) {
  std::ifstream ifs(path.c_str());
  if (!ifs.is_open()) {
    throw std::runtime_error("failed to open user dict: " + path);
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
    entries[tokens[0]] = ParseUserDictValues(tokens, lineNumber);
  }
}

void WriteOcd2(const std::map<std::string, std::vector<std::string>>& entries,
               const std::string& outputPath) {
  std::vector<std::unique_ptr<DictEntry>> lexiconEntries;
  lexiconEntries.reserve(entries.size());
  for (const auto& entry : entries) {
    lexiconEntries.push_back(std::unique_ptr<DictEntry>(
        DictEntryFactory::New(entry.first, entry.second)));
  }

  LexiconDict dict(std::move(lexiconEntries));
  MarisaDictPtr marisaDict = MarisaDict::NewFromDict(dict);
  static_cast<const SerializableDict&>(*marisaDict).SerializeToFile(outputPath);
}

} // namespace

int main(int argc, char** argv) {
  std::vector<std::string> inputPaths;
  std::string outputPath;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "-i") {
      if (i + 1 >= argc) {
        std::cerr << "opencc_jieba_dict_build_tool: missing value after -i"
                  << std::endl;
        PrintUsage(std::cerr);
        return 1;
      }
      inputPaths.push_back(argv[++i]);
      continue;
    }
    if (arg == "-o") {
      if (i + 1 >= argc) {
        std::cerr << "opencc_jieba_dict_build_tool: missing value after -o"
                  << std::endl;
        PrintUsage(std::cerr);
        return 1;
      }
      if (!outputPath.empty()) {
        std::cerr << "opencc_jieba_dict_build_tool: exactly one -o is required"
                  << std::endl;
        PrintUsage(std::cerr);
        return 1;
      }
      outputPath = argv[++i];
      continue;
    }

    std::cerr << "opencc_jieba_dict_build_tool: unknown argument: " << arg
              << std::endl;
    PrintUsage(std::cerr);
    return 1;
  }

  if (inputPaths.empty()) {
    std::cerr << "opencc_jieba_dict_build_tool: at least one -i is required"
              << std::endl;
    PrintUsage(std::cerr);
    return 1;
  }
  if (outputPath.empty()) {
    std::cerr << "opencc_jieba_dict_build_tool: exactly one -o is required"
              << std::endl;
    PrintUsage(std::cerr);
    return 1;
  }

  try {
    std::map<std::string, std::vector<std::string>> entries;
    LoadBaseDict(inputPaths.front(), entries);
    for (size_t i = 1; i < inputPaths.size(); ++i) {
      LoadUserDict(inputPaths[i], entries);
    }
    WriteOcd2(entries, outputPath);
  } catch (const std::exception& e) {
    std::cerr << "opencc_jieba_dict_build_tool: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
