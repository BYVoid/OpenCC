/*
 * Open Chinese Convert
 *
 * Copyright 2015 Carbo Kuo <byvoid@byvoid.com>
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

#include <functional>
#include <unordered_map>

#include "Common.hpp"
#include "UTF8StringSlice.hpp"

namespace opencc {

class OPENCC_EXPORT PhraseExtract {
public:
  typedef UTF8StringSlice::LengthType LengthType;

  typedef UTF8StringSliceBase<unsigned char> UTF8StringSlice8Bit;

  PhraseExtract();

  virtual ~PhraseExtract();

  void Extract(const std::string& text) {
    SetFullText(text);
    ExtractSuffixes();
    CalculateFrequency();
    CalculateSuffixEntropy();
    ReleaseSuffixes();
    ExtractPrefixes();
    CalculatePrefixEntropy();
    ReleasePrefixes();
    ExtractWordCandidates();
    CalculateCohesions();
    SelectWords();
  }

  void SetFullText(const std::string& fullText) {
    utf8FullText = UTF8StringSlice(fullText.c_str());
  }

  void SetFullText(const char* fullText) {
    utf8FullText = UTF8StringSlice(fullText);
  }

  void SetFullText(const UTF8StringSlice& fullText) { utf8FullText = fullText; }

  void SetWordMinLength(const LengthType _wordMinLength) {
    wordMinLength = _wordMinLength;
  }

  void SetWordMaxLength(const LengthType _wordMaxLength) {
    wordMaxLength = _wordMaxLength;
  }

  void SetPrefixSetLength(const LengthType _prefixSetLength) {
    prefixSetLength = _prefixSetLength;
  }

  void SetSuffixSetLength(const LengthType _suffixSetLength) {
    suffixSetLength = _suffixSetLength;
  }

  // PreCalculationFilter is called after frequencies statistics.
  void SetPreCalculationFilter(
      const std::function<bool(const PhraseExtract&,
                               const UTF8StringSlice8Bit&)>& filter) {
    preCalculationFilter = filter;
  }

  void SetPostCalculationFilter(
      const std::function<bool(const PhraseExtract&,
                               const UTF8StringSlice8Bit&)>& filter) {
    postCalculationFilter = filter;
  }

  void ReleaseSuffixes() { std::vector<UTF8StringSlice8Bit>().swap(suffixes); }

  void ReleasePrefixes() { std::vector<UTF8StringSlice8Bit>().swap(prefixes); }

  const std::vector<UTF8StringSlice8Bit>& Words() const { return words; }

  const std::vector<UTF8StringSlice8Bit>& WordCandidates() const {
    return wordCandidates;
  }

  struct Signals {
    size_t frequency;
    double cohesion;
    double suffixEntropy;
    double prefixEntropy;
  };

  const Signals& Signal(const UTF8StringSlice8Bit& wordCandidate) const;

  double Cohesion(const UTF8StringSlice8Bit& wordCandidate) const;

  double Entropy(const UTF8StringSlice8Bit& wordCandidate) const;

  double SuffixEntropy(const UTF8StringSlice8Bit& wordCandidate) const;

  double PrefixEntropy(const UTF8StringSlice8Bit& wordCandidate) const;

  size_t Frequency(const UTF8StringSlice8Bit& word) const;

  double Probability(const UTF8StringSlice8Bit& word) const;

  double LogProbability(const UTF8StringSlice8Bit& word) const;

  void Reset();

  void ExtractSuffixes();

  void ExtractPrefixes();

  void ExtractWordCandidates();

  void CalculateFrequency();

  void CalculateCohesions();

  void CalculateSuffixEntropy();

  void CalculatePrefixEntropy();

  void SelectWords();

  static bool
  DefaultPreCalculationFilter(const PhraseExtract&,
                              const PhraseExtract::UTF8StringSlice8Bit&);

  static bool
  DefaultPostCalculationFilter(const PhraseExtract&,
                               const PhraseExtract::UTF8StringSlice8Bit&);

private:
  class DictType;

  // Pointwise Mutual Information
  double PMI(const UTF8StringSlice8Bit& wordCandidate,
             const UTF8StringSlice8Bit& part1,
             const UTF8StringSlice8Bit& part2) const;

  double CalculateCohesion(const UTF8StringSlice8Bit& wordCandidate) const;

  double CalculateEntropy(
      const std::unordered_map<UTF8StringSlice8Bit, size_t,
                               UTF8StringSlice8Bit::Hasher>& choices) const;

  LengthType wordMinLength;
  LengthType wordMaxLength;
  LengthType prefixSetLength;
  LengthType suffixSetLength;
  std::function<bool(const PhraseExtract&, const UTF8StringSlice8Bit&)>
      preCalculationFilter;
  std::function<bool(const PhraseExtract&, const UTF8StringSlice8Bit&)>
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
  std::vector<UTF8StringSlice8Bit> prefixes;
  std::vector<UTF8StringSlice8Bit> suffixes;
  std::vector<UTF8StringSlice8Bit> wordCandidates;
  std::vector<UTF8StringSlice8Bit> words;
  DictType* signals;

  friend class PhraseExtractTest;
};

} // namespace opencc
