/*
 * Open Chinese Convert
 *
 * Copyright 2015 BYVoid <byvoid@byvoid.com>
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

#pragma once

#include "Common.hpp"
#include "UTF8StringSlice.hpp"

namespace opencc {
namespace tools {

class WordDetection {
public:
  WordDetection(const size_t _wordMaxLength);

  void Detect(const string& fullText);

private:
  void ExtractSuffixes(UTF8StringSlice utf8FullText);

  void CalculateFrequency();

  void CalculateCohesions();

  void CalculateSuffixEntropy();

  double Probability(const UTF8StringSlice& word) const;

  double LogProbability(const UTF8StringSlice& word) const;

  // Pointwise Mutual Information
  double PMI(const UTF8StringSlice& wordCandidate, const UTF8StringSlice& part1,
             const UTF8StringSlice& part2) const;

  double Cohesion(const UTF8StringSlice& wordCandidate) const;

  double Entropy(const std::map<UTF8StringSlice, size_t>& choices) const;

  const size_t wordMaxLength;
  const size_t suffixSetLength = 1;

  vector<UTF8StringSlice> prefixes, suffixes, wordCandidates;
  size_t totalOccurrence;
  double logTotalOccurrence;
  std::map<UTF8StringSlice, size_t> frequencies;
  std::map<UTF8StringSlice, double> cohesions;
  std::map<UTF8StringSlice, double> suffixEntropies;
};
}
}
