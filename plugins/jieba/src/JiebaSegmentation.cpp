/*
 * Open Chinese Convert
 *
 * Copyright 2026 Frank Lin <github@linshuang.info>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "JiebaSegmentation.hpp"
#include "Segments.hpp"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <sys/stat.h>

#include "cppjieba/Jieba.hpp"

#if defined(__has_include)
#if __has_include(<opencc/MarisaDict.hpp>)
#include <opencc/Lexicon.hpp>
#include <opencc/MarisaDict.hpp>
#include <opencc/SerializableDict.hpp>
#elif __has_include("MarisaDict.hpp")
#include "Lexicon.hpp"
#include "MarisaDict.hpp"
#include "SerializableDict.hpp"
#elif __has_include("src/MarisaDict.hpp")
#include "src/Lexicon.hpp"
#include "src/MarisaDict.hpp"
#include "src/SerializableDict.hpp"
#else
#error "Unable to locate OpenCC dictionary headers"
#endif
#else
#include "Lexicon.hpp"
#include "MarisaDict.hpp"
#include "SerializableDict.hpp"
#endif

using namespace opencc;

namespace {
using cppjieba::DictTrie;
using cppjieba::DictUnit;
using opencc::DictEntry;
using opencc::LexiconPtr;
using opencc::MarisaDict;
using opencc::SerializableDict;

std::string ParentDir(const std::string& path) {
  std::string::size_type pos = path.find_last_of("/\\");
  if (pos == std::string::npos) {
    return "";
  }
  return path.substr(0, pos + 1);
}

bool IsRegularFile(const std::string& path) {
  struct stat info;
  if (stat(path.c_str(), &info) != 0) {
    return false;
  }
  return (info.st_mode & S_IFMT) == S_IFREG;
}

std::string ResolveAuxPath(const std::string& dictPath,
                           const std::string& modelPath,
                           const std::string& fileName) {
  const std::string baseDir = ParentDir(dictPath);
  const std::string candidate = baseDir + fileName;
  if (IsRegularFile(candidate)) {
    return candidate;
  }
  const std::string modelDir = ParentDir(modelPath);
  if (!modelDir.empty()) {
    const std::string modelCandidate = modelDir + fileName;
    if (IsRegularFile(modelCandidate)) {
      return modelCandidate;
    }
  }
  // Development fallback: try deps/cppjieba/dict/ relative to the source tree
  const std::string needle = "share/opencc/jieba_dict/";
  std::string::size_type pos = dictPath.find(needle);
  if (pos != std::string::npos) {
    std::string alt = dictPath;
    alt.replace(pos, needle.size(), "plugins/jieba/deps/cppjieba/dict/");
    const std::string altDir = ParentDir(alt);
    const std::string altCandidate = altDir + fileName;
    if (IsRegularFile(altCandidate)) {
      return altCandidate;
    }
  }
  return candidate;
}

bool EndsWith(const std::string& text, const std::string& suffix) {
  return text.size() >= suffix.size() &&
         text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool LooksLikeOpenccMergedDict(const std::string& path) {
  return EndsWith(path, ".ocd2");
}

bool ParsePositiveDouble(const std::string& text, double* value) {
  if (text.empty()) {
    return false;
  }
  char* end = nullptr;
  const double parsed = std::strtod(text.c_str(), &end);
  if (end == text.c_str() || *end != '\0' || parsed <= 0.0) {
    return false;
  }
  *value = parsed;
  return true;
}

struct RawMergedEntry {
  std::string word;
  std::string tag;
  double freq;
  bool contributes_to_base_sum;
  bool uses_default_weight;
};

DictTrie::PrecomputedDict LoadMergedJiebaDict(const std::string& dictPath) {
  const std::shared_ptr<MarisaDict> dict =
      SerializableDict::NewFromFile<MarisaDict>(dictPath);
  const LexiconPtr lexicon = dict->GetLexicon();

  std::vector<RawMergedEntry> entries;
  entries.reserve(lexicon->Length());
  double base_freq_sum = 0.0;
  bool saw_metadata = false;

  for (size_t i = 0; i < lexicon->Length(); ++i) {
    const DictEntry* entry = lexicon->At(i);
    const std::vector<std::string> values = entry->Values();
    RawMergedEntry raw = {entry->Key(), "", 0.0, false, false};

    if (values.size() == 3) {
      saw_metadata = true;
      raw.tag = values[1];
      if (values[2] == "base") {
        if (!ParsePositiveDouble(values[0], &raw.freq)) {
          throw std::runtime_error("invalid merged jieba base frequency for: " +
                                   raw.word);
        }
        raw.contributes_to_base_sum = true;
      } else if (values[2] == "user_freq") {
        if (!ParsePositiveDouble(values[0], &raw.freq)) {
          throw std::runtime_error(
              "invalid merged jieba user frequency for: " + raw.word);
        }
      } else if (values[2] == "user_default") {
        raw.uses_default_weight = true;
      } else {
        throw std::runtime_error("unknown merged jieba entry kind for: " +
                                 raw.word);
      }
    } else if (values.size() == 2) {
      // Backward-compatible fallback for earlier generated dictionaries.
      raw.tag = values[1];
      if (!ParsePositiveDouble(values[0], &raw.freq)) {
        throw std::runtime_error("invalid merged jieba frequency for: " +
                                 raw.word);
      }
      raw.contributes_to_base_sum = true;
    } else if (values.size() == 1) {
      raw.tag = values[0];
      raw.uses_default_weight = true;
    } else {
      throw std::runtime_error("invalid merged jieba values for: " + raw.word);
    }

    if (raw.contributes_to_base_sum) {
      base_freq_sum += raw.freq;
    }
    entries.push_back(raw);
  }

  if (entries.empty()) {
    throw std::runtime_error("merged jieba dictionary is empty");
  }
  if (base_freq_sum <= 0.0) {
    throw std::runtime_error("merged jieba dictionary has no base frequencies");
  }

  std::vector<double> base_weights;
  base_weights.reserve(entries.size());
  for (size_t i = 0; i < entries.size(); ++i) {
    if (entries[i].contributes_to_base_sum) {
      base_weights.push_back(std::log(entries[i].freq / base_freq_sum));
    } else if (!saw_metadata && !entries[i].uses_default_weight &&
               entries[i].freq > 0.0) {
      base_weights.push_back(std::log(entries[i].freq / base_freq_sum));
    }
  }
  if (base_weights.empty()) {
    throw std::runtime_error(
        "merged jieba dictionary has no weights for default entries");
  }

  std::sort(base_weights.begin(), base_weights.end());
  DictTrie::PrecomputedDict precomputed;
  precomputed.freq_sum = base_freq_sum;
  precomputed.min_weight = base_weights.front();
  precomputed.max_weight = base_weights.back();
  precomputed.median_weight = base_weights[base_weights.size() / 2];
  precomputed.node_infos.reserve(entries.size());

  for (size_t i = 0; i < entries.size(); ++i) {
    DictUnit node_info;
    if (!cppjieba::DecodeUTF8RunesInString(entries[i].word, node_info.word)) {
      throw std::runtime_error("UTF-8 decode failed for merged jieba word: " +
                               entries[i].word);
    }
    node_info.tag = entries[i].tag;
    if (entries[i].uses_default_weight) {
      node_info.weight = precomputed.median_weight;
    } else {
      node_info.weight = std::log(entries[i].freq / base_freq_sum);
    }
    precomputed.node_infos.push_back(node_info);
  }

  return precomputed;
}
} // namespace

JiebaSegmentation::JiebaSegmentation(const std::string& dictPath,
                                     const std::string& modelPath,
                                     const std::string& userDictPath)
    : jieba_(
          LooksLikeOpenccMergedDict(dictPath)
              ? new cppjieba::Jieba(LoadMergedJiebaDict(dictPath), modelPath,
                                    userDictPath.empty() ? "" : userDictPath,
                                    ResolveAuxPath(dictPath, modelPath, "idf.utf8"),
                                    ResolveAuxPath(dictPath, modelPath, "stop_words.utf8"))
              : new cppjieba::Jieba(
                    dictPath, modelPath,
                    userDictPath.empty() ? "" : userDictPath,
                    ResolveAuxPath(dictPath, modelPath, "idf.utf8"),
                    ResolveAuxPath(dictPath, modelPath, "stop_words.utf8"))) {
}

JiebaSegmentation::~JiebaSegmentation() = default;

SegmentsPtr JiebaSegmentation::Segment(const std::string& text) const {
  SegmentsPtr segments(new Segments);
  std::vector<std::string> words;
  jieba_->Cut(text, words, true);
  for (const auto& word : words) {
    segments->AddSegment(word);
  }
  return segments;
}
