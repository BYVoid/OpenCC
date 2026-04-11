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

#include <sys/stat.h>

#include "cppjieba/Jieba.hpp"

using namespace opencc;

namespace {
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
                           const std::string& fileName) {
  const std::string baseDir = ParentDir(dictPath);
  const std::string candidate = baseDir + fileName;
  if (IsRegularFile(candidate)) {
    return candidate;
  }
  const std::string needle = "data/jieba_dict/";
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
} // namespace

JiebaSegmentation::JiebaSegmentation(const std::string& dictPath,
                                     const std::string& modelPath,
                                     const std::string& userDictPath)
    : jieba_(new cppjieba::Jieba(
          dictPath, modelPath, userDictPath.empty() ? "" : userDictPath,
          ResolveAuxPath(dictPath, "idf.utf8"),
          ResolveAuxPath(dictPath, "stop_words.utf8"))) {
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
