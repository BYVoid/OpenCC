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

#include <unordered_map>

#include "Common.hpp"
#include "UTF8StringSlice.hpp"

namespace opencc {

class PhraseExtract {
public:
  PhraseExtract();

  virtual ~PhraseExtract();

  void Extract(const string& text) {
    SetFullText(text);
    ExtractSuffixes();
    ExtractPrefixes();
    CalculateFrequency();
    CalculateSuffixEntropy();
    CalculatePrefixEntropy();
    CalculateCohesions();
    SelectWords();
  }

  void SetFullText(const string& fullText) {
    utf8FullText = UTF8StringSlice(fullText.c_str());
  }

  void SetFullText(const char* fullText) {
    utf8FullText = UTF8StringSlice(fullText);
  }

  void SetFullText(const UTF8StringSlice& fullText) { utf8FullText = fullText; }

  void SetWordMinLength(const size_t _wordMinLength) {
    wordMinLength = _wordMinLength;
  }

  void SetWordMaxLength(const size_t _wordMaxLength) {
    wordMaxLength = _wordMaxLength;
  }

  void SetPrefixSetLength(const size_t _prefixSetLength) {
    prefixSetLength = _prefixSetLength;
  }

  void SetSuffixSetLength(const size_t _suffixSetLength) {
    suffixSetLength = _suffixSetLength;
  }

  void SetPreCalculationFilter(const std::function<
      bool(const PhraseExtract&, const UTF8StringSlice&)>& filter) {
    preCalculationFilter = filter;
  }

  void SetPostCalculationFilter(const std::function<
      bool(const PhraseExtract&, const UTF8StringSlice&)>& filter) {
    postCalculationFilter = filter;
  }

  const vector<UTF8StringSlice>& Words() const { return words; }

  const vector<UTF8StringSlice>& WordCandidates() const {
    return wordCandidates;
  }

  struct Signals {
    size_t frequency;
    double cohesion;
    double suffixEntropy;
    double prefixEntropy;
  };

  const Signals& Signal(const UTF8StringSlice& wordCandidate) const;

  double Cohesion(const UTF8StringSlice& wordCandidate) const;

  double Entropy(const UTF8StringSlice& wordCandidate) const;

  double SuffixEntropy(const UTF8StringSlice& wordCandidate) const;

  double PrefixEntropy(const UTF8StringSlice& wordCandidate) const;

  size_t Frequency(const UTF8StringSlice& word) const;

  double LogProbability(const UTF8StringSlice& word) const;

  void Reset();

  void ExtractSuffixes();

  void ExtractPrefixes();

  void ExtractWordCandidates();

  void CalculateFrequency();

  void CalculateCohesions();

  void CalculateSuffixEntropy();

  void CalculatePrefixEntropy();

  void SelectWords();

private:
  class DictType;

  // Pointwise Mutual Information
  double PMI(const UTF8StringSlice& wordCandidate, const UTF8StringSlice& part1,
             const UTF8StringSlice& part2) const;

  double CalculateCohesion(const UTF8StringSlice& wordCandidate) const;

  double CalculateEntropy(const std::unordered_map<
      UTF8StringSlice, size_t, UTF8StringSlice::Hasher>& choices) const;

  size_t wordMinLength;
  size_t wordMaxLength;
  size_t prefixSetLength;
  size_t suffixSetLength;
  std::function<bool(const PhraseExtract&, const UTF8StringSlice&)>
      preCalculationFilter;
  std::function<bool(const PhraseExtract&, const UTF8StringSlice&)>
      postCalculationFilter;

  bool prefixesExtracted;
  bool suffixesExtracted;
  bool frequenciesCalculated;
  bool wordCandidatesExtracted;
  bool cohesionsCalculated;
  bool prefixEntropiesCalculated;
  bool suffixEntropiesCalculated;
  bool wordsSelected;

  UTF8StringSlice utf8FullText;
  size_t totalOccurrence;
  double logTotalOccurrence;
  vector<UTF8StringSlice> prefixes;
  vector<UTF8StringSlice> suffixes;
  vector<UTF8StringSlice> wordCandidates;
  vector<UTF8StringSlice> words;
  DictType* signals;

  friend class PhraseExtractTest;
};

} // namespace opencc
