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

  void SetPreCalculationFilter(
      const std::function<bool(const UTF8StringSlice& word)>& filter) {
    preCalculationFilter = filter;
  }

  void SetPostCalculationFilter(
      const std::function<bool(const UTF8StringSlice& word)>& filter) {
    postCalculationFilter = filter;
  }

  const vector<UTF8StringSlice>& Words() const { return words; }

  const vector<UTF8StringSlice>& WordCandidates() const {
    return wordCandidates;
  }

  double Cohesion(const UTF8StringSlice& wordCandidate) const;

  double Entropy(const UTF8StringSlice& wordCandidate) const;

  double SuffixEntropy(const UTF8StringSlice& wordCandidate) const;

  double PrefixEntropy(const UTF8StringSlice& wordCandidate) const;

  size_t Frequency(const UTF8StringSlice& word) const;

  double LogProbability(const UTF8StringSlice& word) const;

private:
  void ExtractSuffixes();

  void ExtractPrefixes();

  void ExtractWordCandidates();

  void CalculateFrequency();

  void CalculateCohesions();

  void CalculateSuffixEntropy();

  void CalculatePrefixEntropy();

  void SelectWords();

  // Pointwise Mutual Information
  double PMI(const UTF8StringSlice& wordCandidate, const UTF8StringSlice& part1,
             const UTF8StringSlice& part2) const;

  double CalculateCohesion(const UTF8StringSlice& wordCandidate) const;

  double
  CalculateEntropy(const std::map<UTF8StringSlice, size_t>& choices) const;

  const size_t wordMaxLength;
  const size_t prefixSetLength;
  const size_t suffixSetLength;
  std::function<bool(const UTF8StringSlice& word)> preCalculationFilter;
  std::function<bool(const UTF8StringSlice& word)> postCalculationFilter;

  UTF8StringSlice utf8FullText;
  size_t totalOccurrence;
  double logTotalOccurrence;
  vector<UTF8StringSlice> prefixes;
  vector<UTF8StringSlice> suffixes;
  vector<UTF8StringSlice> wordCandidates;
  vector<UTF8StringSlice> words;
  std::map<UTF8StringSlice, size_t> frequencies;
  std::map<UTF8StringSlice, double> cohesions;
  std::map<UTF8StringSlice, double> suffixEntropies;
  std::map<UTF8StringSlice, double> prefixEntropies;
};

} // namespace tools
} // namespace opencc
