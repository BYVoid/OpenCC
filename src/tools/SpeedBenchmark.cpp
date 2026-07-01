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

// Throughput benchmark for SimpleConverter.
//
// Usage:
//   speed_benchmark [--resource-zip <zip>] <config> <corpus_file> [reps]
//                   [-- <search_path>...]
//
// Examples:
//   speed_benchmark s2t.json /tmp/corpus.txt 30 -- data/config bazel-bin/data/dictionary
//   speed_benchmark --resource-zip opencc-resources-ocd2.zip s2t.json /tmp/corpus.txt 30
//
// Prints:
//   - init time (ms)
//   - corpus size
//   - per-iteration time (ms) and throughput (M chars/s, ns/char)

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "src/ResourceProvider.hpp"
#include "src/SimpleConverter.hpp"
#include "src/UTF8Util.hpp"

using Clock = std::chrono::steady_clock;

static double ElapsedMs(Clock::time_point a, Clock::time_point b) {
  return std::chrono::duration<double, std::milli>(b - a).count();
}

static std::string ReadFile(const std::string& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    throw std::runtime_error("cannot open file: " + path);
  }
  return std::string(std::istreambuf_iterator<char>(f), {});
}

int main(int argc, char** argv) {
  std::string resourceZip;
  std::vector<std::string> extraPaths;

  // Collect non-positional args first
  int argIdx = 1;
  while (argIdx < argc && std::string(argv[argIdx]) == "--resource-zip") {
    if (argIdx + 1 >= argc) {
      std::cerr << "error: --resource-zip requires an argument\n";
      return 1;
    }
    resourceZip = argv[argIdx + 1];
    argIdx += 2;
  }

  if (argIdx + 1 >= argc) {
    std::cerr << "usage: speed_benchmark [--resource-zip <zip>] <config> <corpus_file>"
                 " [reps] [-- <search_path>...]\n";
    return 1;
  }

  const std::string config     = argv[argIdx++];
  const std::string corpusFile = argv[argIdx++];
  int               reps       = 20;

  for (int i = argIdx; i < argc; i++) {
    if (std::string(argv[i]) == "--") {
      for (int j = i + 1; j < argc; j++) {
        extraPaths.push_back(argv[j]);
      }
      break;
    }
    reps = std::stoi(argv[i]);
  }

  const std::string corpus = ReadFile(corpusFile);
  const size_t corpusBytes = corpus.size();

  // Count Unicode code points for a fair comparison with JS benchmark.
  size_t corpusChars = 0;
  for (const char* p = corpus.data(), *end = p + corpus.size(); p < end;) {
    const size_t n = opencc::UTF8Util::NextCharLength(p);
    p += (n > 0 && p + n <= end) ? n : 1;
    ++corpusChars;
  }

  // ── init ──────────────────────────────────────────────────────────────────
  auto t0 = Clock::now();
  std::unique_ptr<opencc::SimpleConverter> converterPtr;
  if (!resourceZip.empty()) {
    auto provider = std::make_shared<opencc::ZipResourceProvider>(resourceZip);
    converterPtr = std::make_unique<opencc::SimpleConverter>(config, provider);
  } else {
    converterPtr = std::make_unique<opencc::SimpleConverter>(config, extraPaths);
  }
  auto t1 = Clock::now();
  const double initMs = ElapsedMs(t0, t1);

  opencc::SimpleConverter& converter = *converterPtr;

  // ── warmup ────────────────────────────────────────────────────────────────
  volatile size_t sink = 0;
  sink += converter.Convert(corpus).size();

  // ── timed loop ────────────────────────────────────────────────────────────
  auto t2 = Clock::now();
  for (int i = 0; i < reps; i++) {
    sink += converter.Convert(corpus).size();
  }
  auto t3 = Clock::now();

  (void)sink;

  const double totalMs  = ElapsedMs(t2, t3);
  const double perIterMs = totalMs / reps;
  const double totalChars = static_cast<double>(corpusChars) * reps;
  const double throughputMCharsPerSec = totalChars / totalMs / 1000.0;
  const double nsPerChar = totalMs * 1e6 / totalChars;

  std::string zipLabel = resourceZip.empty() ? "(filesystem)" : resourceZip;
  // Strip directory prefix for brevity
  const size_t slash = zipLabel.rfind('/');
  if (slash != std::string::npos) zipLabel = zipLabel.substr(slash + 1);

  std::cout << "C++ SimpleConverter  config=" << config
            << "  zip=" << zipLabel << "\n";
  std::cout << "  init:         " << initMs       << " ms\n";
  std::cout << "  corpus:       " << corpusChars  << " chars  "
                                  << corpusBytes  << " bytes\n";
  std::cout << "  reps:         " << reps         << "\n";
  std::cout << "  per iter:     " << perIterMs    << " ms\n";
  std::cout << "  throughput:   " << throughputMCharsPerSec << " M chars/s\n";
  std::cout << "  ns/char:      " << nsPerChar    << "\n";

  return 0;
}
