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

#include <cmath>
#include <unordered_map>
#include "darts.h"

#include "PhraseExtract.hpp"

namespace opencc {

namespace internal {

bool ContainsPunctuation(const PhraseExtract::UTF8StringSlice8Bit& word) {
  static const vector<PhraseExtract::UTF8StringSlice8Bit> punctuations = {
      " ",  "\n", "\r", "\t", "-",  ",",  ".",  "?",  "!",  "*", "　",
      "，", "。", "、", "；", "：", "？", "！", "…",  "“",  "”", "「",
      "」", "—",  "－", "（", "）", "《", "》", "．", "／", "＼"};
  for (const auto& punctuation : punctuations) {
    if (word.FindBytePosition(punctuation) !=
        static_cast<PhraseExtract::UTF8StringSlice8Bit::LengthType>(-1)) {
      return true;
    }
  }
  return false;
}

} // namespace internal

class PhraseExtract::DictType {
public:
  typedef PhraseExtract::Signals ValueType;
  typedef std::pair<UTF8StringSlice8Bit, ValueType> ItemType;

  PhraseExtract::Signals& Get(const UTF8StringSlice8Bit& key) {
    Darts::DoubleArray::result_pair_type result;
    daTrie.exactMatchSearch(key.CString(), result, key.ByteLength());
    if (result.value != -1) {
      return items[result.value].second;
    } else {
      throw ShouldNotBeHere();
    }
  }

  PhraseExtract::Signals& AddKey(const UTF8StringSlice8Bit& key) {
    return dict[key];
  }

  void Clear() {
    ClearDict();
    daTrie.clear();
  }

  const vector<ItemType>& Items() const { return items; }

  void Build() {
    BuildKeys();
    BuildDaTrie();
  }

private:
  void BuildKeys() {
    items.reserve(dict.size());
    for (const auto& item : dict) {
      items.push_back(item);
    }
    ClearDict();
    std::sort(
        items.begin(), items.end(),
        [](const ItemType& a, const ItemType& b) { return a.first < b.first; });
  }

  void ClearDict() {
    std::unordered_map<UTF8StringSlice8Bit, PhraseExtract::Signals,
                       UTF8StringSlice8Bit::Hasher>().swap(dict);
  }

  void BuildDaTrie() {
    const char** keyNames = new const char* [items.size()];
    size_t* keyLengths = new size_t[items.size()];
    for (size_t i = 0; i < items.size(); i++) {
      const auto& key = items[i].first;
      keyNames[i] = key.CString();
      keyLengths[i] = key.ByteLength();
    }
    daTrie.build(items.size(), keyNames, keyLengths);
    delete[] keyNames;
    delete[] keyLengths;
  }

  std::unordered_map<UTF8StringSlice8Bit, PhraseExtract::Signals,
                     UTF8StringSlice8Bit::Hasher> dict;
  vector<ItemType> items;
  Darts::DoubleArray daTrie;
};

using namespace internal;

bool PhraseExtract::DefaultPreCalculationFilter(
    const PhraseExtract&, const PhraseExtract::UTF8StringSlice8Bit&) {
  return false;
}

bool PhraseExtract::DefaultPostCalculationFilter(
    const PhraseExtract& phraseExtract,
    const PhraseExtract::UTF8StringSlice8Bit& word) {
  const PhraseExtract::Signals& signals = phraseExtract.Signal(word);
  const double logProbability = phraseExtract.LogProbability(word);
  const double cohesionScore = signals.cohesion - logProbability * 0.5;
  const double entropyScore =
      sqrt((signals.prefixEntropy) * (signals.suffixEntropy + 1)) -
      logProbability * 0.85;
  bool accept = cohesionScore > 9 && entropyScore > 11 &&
                signals.prefixEntropy > 0.5 && signals.suffixEntropy > 0 &&
                signals.prefixEntropy + signals.suffixEntropy > 3;
  return !accept;
}

PhraseExtract::PhraseExtract()
    : wordMinLength(2), wordMaxLength(2), prefixSetLength(1),
      suffixSetLength(1), preCalculationFilter(DefaultPreCalculationFilter),
      postCalculationFilter(DefaultPostCalculationFilter), utf8FullText(""),
      signals(new DictType) {
  Reset();
}

PhraseExtract::~PhraseExtract() { delete signals; }

void PhraseExtract::Reset() {
  prefixesExtracted = false;
  suffixesExtracted = false;
  frequenciesCalculated = false;
  wordCandidatesExtracted = false;
  cohesionsCalculated = false;
  prefixEntropiesCalculated = false;
  suffixEntropiesCalculated = false;
  wordsSelected = false;
  totalOccurrence = 0;
  logTotalOccurrence = 0;
  ReleasePrefixes();
  ReleaseSuffixes();
  wordCandidates.clear();
  words.clear();
  signals->Clear();
  utf8FullText = UTF8StringSlice("");
  preCalculationFilter = DefaultPreCalculationFilter;
  postCalculationFilter = DefaultPostCalculationFilter;
}

void PhraseExtract::ExtractSuffixes() {
  suffixes.reserve(utf8FullText.UTF8Length() / 2 *
                   (wordMaxLength + suffixSetLength));
  for (UTF8StringSlice text = utf8FullText; text.UTF8Length() > 0;
       text.MoveRight()) {
    const LengthType suffixLength =
        std::min(static_cast<LengthType>(wordMaxLength + suffixSetLength),
                 text.UTF8Length());
    const UTF8StringSlice& slice = text.Left(suffixLength);
    suffixes.push_back(UTF8StringSlice8Bit(slice.CString(), 
        static_cast<UTF8StringSlice8Bit::LengthType>(slice.UTF8Length()),
        static_cast<UTF8StringSlice8Bit::LengthType>(slice.ByteLength())));
  }
  suffixes.shrink_to_fit();
  // Sort suffixes
  std::sort(suffixes.begin(), suffixes.end());
  suffixesExtracted = true;
}

void PhraseExtract::ExtractPrefixes() {
  prefixes.reserve(utf8FullText.UTF8Length() / 2 *
                   (wordMaxLength + prefixSetLength));
  for (UTF8StringSlice text = utf8FullText; text.UTF8Length() > 0;
       text.MoveLeft()) {
    const LengthType prefixLength =
        std::min(static_cast<LengthType>(wordMaxLength + prefixSetLength),
                 text.UTF8Length());
    const UTF8StringSlice& slice = text.Right(prefixLength);
    prefixes.push_back(UTF8StringSlice8Bit(slice.CString(),
        static_cast<UTF8StringSlice8Bit::LengthType>(slice.UTF8Length()),
        static_cast<UTF8StringSlice8Bit::LengthType>(slice.ByteLength())));

  }
  prefixes.shrink_to_fit();
  // Sort suffixes reversely
  std::sort(prefixes.begin(), prefixes.end(),
            [](const UTF8StringSlice8Bit& a, const UTF8StringSlice8Bit& b) {
    return a.ReverseCompare(b) < 0;
  });
  prefixesExtracted = true;
}

void PhraseExtract::CalculateFrequency() {
  if (!suffixesExtracted) {
    ExtractSuffixes();
  }
  for (const auto& suffix : suffixes) {
    for (UTF8StringSlice8Bit::LengthType i = 1; i <= suffix.UTF8Length() && i <= wordMaxLength; i++) {
      const UTF8StringSlice8Bit wordCandidate = suffix.Left(i);
      signals->AddKey(wordCandidate).frequency++;
      totalOccurrence++;
    }
  }
  logTotalOccurrence = log(totalOccurrence);
  signals->Build();
  frequenciesCalculated = true;
}

void PhraseExtract::ExtractWordCandidates() {
  if (!frequenciesCalculated) {
    CalculateFrequency();
  }
  for (const auto& item : signals->Items()) {
    const auto& wordCandidate = item.first;
    if (wordCandidate.UTF8Length() < wordMinLength) {
      continue;
    }
    if (ContainsPunctuation(wordCandidate)) {
      continue;
    }
    if (preCalculationFilter(*this, wordCandidate)) {
      continue;
    }
    wordCandidates.push_back(wordCandidate);
  }
  // Sort by frequency
  std::sort(wordCandidates.begin(), wordCandidates.end(),
            [this](const UTF8StringSlice8Bit& a, const UTF8StringSlice8Bit& b) {
    const size_t freqA = Frequency(a);
    const size_t freqB = Frequency(b);
    if (freqA > freqB) {
      return true;
    } else if (freqA < freqB) {
      return false;
    } else {
      return a < b;
    }
  });
  wordCandidatesExtracted = true;
}

typedef std::unordered_map<PhraseExtract::UTF8StringSlice8Bit, size_t,
                           PhraseExtract::UTF8StringSlice8Bit::Hasher>
    AdjacentSetType;

template <bool SUFFIX>
void CalculatePrefixSuffixEntropy(
    const vector<PhraseExtract::UTF8StringSlice8Bit>& presuffixes,
    const PhraseExtract::LengthType setLength,
    const PhraseExtract::LengthType wordMinLength,
    const PhraseExtract::LengthType wordMaxLength,
    const std::function<void(const PhraseExtract::UTF8StringSlice8Bit& word,
                             AdjacentSetType& adjacentSet)>& updateEntropy) {
  AdjacentSetType adjacentSet;
  auto setLength8Bit = static_cast<PhraseExtract::UTF8StringSlice8Bit::LengthType>(setLength);
  for (PhraseExtract::LengthType length = wordMinLength;
       length <= wordMaxLength; length++) {
    adjacentSet.clear();
    PhraseExtract::UTF8StringSlice8Bit lastWord("");
    for (const auto& presuffix : presuffixes) {
      if (presuffix.UTF8Length() < length) {
        continue;
      }
      auto length8Bit = static_cast<PhraseExtract::UTF8StringSlice8Bit::LengthType>(length);
      const auto& wordCandidate =
          SUFFIX ? presuffix.Left(length8Bit) : presuffix.Right(length8Bit);
      if (wordCandidate != lastWord) {
        updateEntropy(lastWord, adjacentSet);
        lastWord = wordCandidate;
      }
      if (length + setLength <= presuffix.UTF8Length()) {
        if (SUFFIX) {
          const auto& wordSuffix = presuffix.SubString(length8Bit, setLength8Bit);
          adjacentSet[wordSuffix]++;
        } else {
          const auto& wordPrefix = presuffix.SubString(
              presuffix.UTF8Length() - length8Bit - setLength8Bit, setLength8Bit);
          adjacentSet[wordPrefix]++;
        }
      }
    }
    updateEntropy(lastWord, adjacentSet);
  }
}

void PhraseExtract::CalculateSuffixEntropy() {
  if (!suffixesExtracted) {
    ExtractSuffixes();
  }
  if (!frequenciesCalculated) {
    CalculateFrequency();
  }
  CalculatePrefixSuffixEntropy<true>(
      suffixes, suffixSetLength, wordMinLength, wordMaxLength,
      [this](const PhraseExtract::UTF8StringSlice8Bit& word,
             AdjacentSetType& adjacentSet) {
        if (word.UTF8Length() > 0) {
          signals->Get(word).suffixEntropy = CalculateEntropy(adjacentSet);
          adjacentSet.clear();
        }
      });
  suffixEntropiesCalculated = true;
}

void PhraseExtract::CalculatePrefixEntropy() {
  if (!prefixesExtracted) {
    ExtractPrefixes();
  }
  if (!frequenciesCalculated) {
    CalculateFrequency();
  }
  CalculatePrefixSuffixEntropy<false>(
      prefixes, prefixSetLength, wordMinLength, wordMaxLength,
      [this](const PhraseExtract::UTF8StringSlice8Bit& word,
             AdjacentSetType& adjacentSet) {
        if (word.UTF8Length() > 0) {
          signals->Get(word).prefixEntropy = CalculateEntropy(adjacentSet);
          adjacentSet.clear();
        }
      });
  prefixEntropiesCalculated = true;
}

void PhraseExtract::CalculateCohesions() {
  if (!wordCandidatesExtracted) {
    ExtractWordCandidates();
  }
  if (!frequenciesCalculated) {
    CalculateFrequency();
  }
  for (const auto& wordCandidate : wordCandidates) {
    signals->Get(wordCandidate).cohesion = CalculateCohesion(wordCandidate);
  }
  cohesionsCalculated = true;
}

const PhraseExtract::Signals&
PhraseExtract::Signal(const UTF8StringSlice8Bit& wordCandidate) const {
  return signals->Get(wordCandidate);
}

double PhraseExtract::Cohesion(const UTF8StringSlice8Bit& word) const {
  return Signal(word).cohesion;
}

double PhraseExtract::Entropy(const UTF8StringSlice8Bit& word) const {
  return SuffixEntropy(word) + PrefixEntropy(word);
}

double PhraseExtract::SuffixEntropy(const UTF8StringSlice8Bit& word) const {
  return Signal(word).suffixEntropy;
}

double PhraseExtract::PrefixEntropy(const UTF8StringSlice8Bit& word) const {
  return Signal(word).prefixEntropy;
}

size_t PhraseExtract::Frequency(const UTF8StringSlice8Bit& word) const {
  const size_t frequency = Signal(word).frequency;
  return frequency;
}

double PhraseExtract::Probability(const UTF8StringSlice8Bit& word) const {
  const size_t frequency = Frequency(word);
  return static_cast<double>(frequency) / totalOccurrence;
}

double PhraseExtract::LogProbability(const UTF8StringSlice8Bit& word) const {
  // log(frequency / totalOccurrence) = log(frequency) - log(totalOccurrence)
  const size_t frequency = Frequency(word);
  return log(frequency) - logTotalOccurrence;
}

double PhraseExtract::PMI(const UTF8StringSlice8Bit& wordCandidate,
                          const UTF8StringSlice8Bit& part1,
                          const UTF8StringSlice8Bit& part2) const {
  // PMI(x, y) = log(P(x, y) / (P(x) * P(y)))
  //           = log(P(x, y)) - log(P(x)) - log(P(y))
  return LogProbability(wordCandidate) - LogProbability(part1) -
         LogProbability(part2);
}

double PhraseExtract::CalculateCohesion(
    const UTF8StringSlice8Bit& wordCandidate) const {
  // TODO Try average value
  double minPMI = INFINITY;
  for (UTF8StringSlice8Bit::LengthType leftLength = 1; leftLength <= wordCandidate.UTF8Length() - 1;
       leftLength++) {
    const auto& leftPart = wordCandidate.Left(leftLength);
    const auto& rightPart =
        wordCandidate.Right(wordCandidate.UTF8Length() - leftLength);
    double pmi = PMI(wordCandidate, leftPart, rightPart);
    minPMI = std::min(pmi, minPMI);
  }
  return minPMI;
}

double PhraseExtract::CalculateEntropy(const std::unordered_map<
    UTF8StringSlice8Bit, size_t, UTF8StringSlice8Bit::Hasher>& choices) const {
  double totalChoices = 0;
  for (const auto& item : choices) {
    totalChoices += item.second;
  }
  double entropy = 0;
  for (const auto& item : choices) {
    const size_t occurrence = item.second;
    const double probability = occurrence / totalChoices;
    entropy += probability * log(probability);
  }
  if (entropy != 0) {
    entropy = -entropy;
  }
  return entropy;
}

void PhraseExtract::SelectWords() {
  if (!wordCandidatesExtracted) {
    ExtractWordCandidates();
  }
  if (!cohesionsCalculated) {
    CalculateCohesions();
  }
  if (!prefixEntropiesCalculated) {
    CalculatePrefixEntropy();
  }
  if (!suffixEntropiesCalculated) {
    CalculateSuffixEntropy();
  }
  for (const auto& word : wordCandidates) {
    if (!postCalculationFilter(*this, word)) {
      words.push_back(word);
    }
  }
  wordsSelected = true;
}

} // namespace opencc
