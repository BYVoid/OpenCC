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
#if __has_include(<opencc/Dict.hpp>)
#include <opencc/Dict.hpp>
#include <opencc/DictEntry.hpp>
#include <opencc/Lexicon.hpp>
#include <opencc/MarisaDict.hpp>
#include <opencc/SerializableDict.hpp>
#elif __has_include("src/Dict.hpp")
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
  os << "Usage: cppjieba_dict -i <dict> [-i <dict> ...] -o <output.ocd2>"
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
    if (ch == '#') {
      return true;
    }
    if (!std::isspace(ch)) {
      return false;
    }
  }
  return true;
}

std::vector<std::string> ParseBaseDictValues(const std::vector<std::string>& tokens,
                                             size_t line_number) {
  if (tokens.size() != 3) {
    throw std::runtime_error("invalid base dict line " +
                             std::to_string(line_number));
  }
  return std::vector<std::string>{tokens[1], tokens[2]};
}

std::vector<std::string> ParseUserDictValues(const std::vector<std::string>& tokens,
                                             size_t line_number) {
  if (tokens.empty() || tokens.size() > 3) {
    throw std::runtime_error("invalid user dict line " +
                             std::to_string(line_number));
  }
  if (tokens.size() == 1) {
    return std::vector<std::string>{""};
  }
  if (tokens.size() == 2) {
    return std::vector<std::string>{tokens[1]};
  }
  return std::vector<std::string>{tokens[1], tokens[2]};
}

class LexiconDict : public Dict {
public:
  explicit LexiconDict(std::vector<std::unique_ptr<DictEntry> > entries)
      : lexicon_(new Lexicon(std::move(entries))), max_length_(0) {
    for (size_t i = 0; i < lexicon_->Length(); ++i) {
      const DictEntry* entry = lexicon_->At(i);
      if (entry->KeyLength() > max_length_) {
        max_length_ = entry->KeyLength();
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
    const size_t capped_len = len < max_length_ ? len : max_length_;
    const DictEntry* best = nullptr;
    size_t best_length = 0;
    for (size_t i = 0; i < lexicon_->Length(); ++i) {
      const DictEntry* entry = lexicon_->At(i);
      const size_t entry_length = entry->KeyLength();
      if (entry_length > capped_len || entry_length <= best_length) {
        continue;
      }
      if (std::string(word, entry_length) == entry->Key()) {
        best = entry;
        best_length = entry_length;
      }
    }
    if (best == nullptr) {
      return Optional<const DictEntry*>::Null();
    }
    return Optional<const DictEntry*>(best);
  }

  std::vector<const DictEntry*> MatchAllPrefixes(const char* word,
                                                 size_t len) const override {
    const size_t capped_len = len < max_length_ ? len : max_length_;
    std::vector<const DictEntry*> matches;
    for (size_t i = 0; i < lexicon_->Length(); ++i) {
      const DictEntry* entry = lexicon_->At(i);
      const size_t entry_length = entry->KeyLength();
      if (entry_length > capped_len) {
        continue;
      }
      if (std::string(word, entry_length) == entry->Key()) {
        matches.push_back(entry);
      }
    }
    return matches;
  }

  size_t KeyMaxLength() const override { return max_length_; }

  LexiconPtr GetLexicon() const override { return lexicon_; }

private:
  LexiconPtr lexicon_;
  size_t max_length_;
};

void LoadBaseDict(const std::string& path,
                  std::map<std::string, std::vector<std::string> >& entries) {
  std::ifstream ifs(path.c_str());
  if (!ifs.is_open()) {
    throw std::runtime_error("failed to open base dict: " + path);
  }

  std::string line;
  size_t line_number = 0;
  while (std::getline(ifs, line)) {
    ++line_number;
    if (IsCommentOrEmpty(line)) {
      continue;
    }
    const std::vector<std::string> tokens = SplitWhitespace(line);
    entries[tokens[0]] = ParseBaseDictValues(tokens, line_number);
  }
}

void LoadUserDict(const std::string& path,
                  std::map<std::string, std::vector<std::string> >& entries) {
  std::ifstream ifs(path.c_str());
  if (!ifs.is_open()) {
    throw std::runtime_error("failed to open user dict: " + path);
  }

  std::string line;
  size_t line_number = 0;
  while (std::getline(ifs, line)) {
    ++line_number;
    if (IsCommentOrEmpty(line)) {
      continue;
    }
    const std::vector<std::string> tokens = SplitWhitespace(line);
    if (tokens.empty()) {
      continue;
    }
    entries[tokens[0]] = ParseUserDictValues(tokens, line_number);
  }
}

void WriteOcd2(const std::map<std::string, std::vector<std::string> >& entries,
               const std::string& output_path) {
  std::vector<std::unique_ptr<DictEntry> > lexicon_entries;
  lexicon_entries.reserve(entries.size());
  for (std::map<std::string, std::vector<std::string> >::const_iterator it =
           entries.begin();
       it != entries.end(); ++it) {
    lexicon_entries.push_back(
        std::unique_ptr<DictEntry>(DictEntryFactory::New(it->first, it->second)));
  }

  LexiconDict dict(std::move(lexicon_entries));
  MarisaDictPtr marisa_dict = MarisaDict::NewFromDict(dict);
  static_cast<const SerializableDict&>(*marisa_dict).SerializeToFile(output_path);
}

}  // namespace

int main(int argc, char** argv) {
  std::vector<std::string> input_paths;
  std::string output_path;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "-i") {
      if (i + 1 >= argc) {
        std::cerr << "cppjieba_dict: missing value after -i" << std::endl;
        PrintUsage(std::cerr);
        return 1;
      }
      input_paths.push_back(argv[++i]);
      continue;
    }
    if (arg == "-o") {
      if (i + 1 >= argc) {
        std::cerr << "cppjieba_dict: missing value after -o" << std::endl;
        PrintUsage(std::cerr);
        return 1;
      }
      if (!output_path.empty()) {
        std::cerr << "cppjieba_dict: exactly one -o is required" << std::endl;
        PrintUsage(std::cerr);
        return 1;
      }
      output_path = argv[++i];
      continue;
    }

    std::cerr << "cppjieba_dict: unknown argument: " << arg << std::endl;
    PrintUsage(std::cerr);
    return 1;
  }

  if (input_paths.empty()) {
    std::cerr << "cppjieba_dict: at least one -i is required" << std::endl;
    PrintUsage(std::cerr);
    return 1;
  }
  if (output_path.empty()) {
    std::cerr << "cppjieba_dict: exactly one -o is required" << std::endl;
    PrintUsage(std::cerr);
    return 1;
  }

  try {
    std::map<std::string, std::vector<std::string> > entries;
    LoadBaseDict(input_paths.front(), entries);
    for (size_t i = 1; i < input_paths.size(); ++i) {
      LoadUserDict(input_paths[i], entries);
    }
    WriteOcd2(entries, output_path);
    std::cout << "Wrote " << entries.size() << " entries to " << output_path
              << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "cppjieba_dict: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
