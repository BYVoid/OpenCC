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

#include "WordDetection.hpp"

namespace opencc {
namespace tools {

WordDetection::WordDetection(const size_t _wordMaxLength)
    : wordMaxLength(_wordMaxLength), totalOccurrence(0) {}

void WordDetection::Detect(const string& fullText) {
  UTF8StringSlice utf8FullText(fullText.c_str());
  ExtractSuffixes(utf8FullText);
  CalculateFrequency();
  CalculateSuffixEntropy();
  CalculateCohesions();
}

void WordDetection::ExtractSuffixes(UTF8StringSlice utf8FullText) {
  // Add all suffixes to candidates
  for (; utf8FullText.UTF8Length() > 0; ++utf8FullText) {
    size_t suffixLength =
        std::min(wordMaxLength + suffixSetLength, utf8FullText.UTF8Length());
    suffixes.push_back(utf8FullText.Left(suffixLength));
  }
  // Sort suffixes
  std::sort(suffixes.begin(), suffixes.end());
}

void WordDetection::CalculateSuffixEntropy() {
  for (size_t length = 1; length <= wordMaxLength; length++) {
    std::map<UTF8StringSlice, size_t> suffixSet;
    UTF8StringSlice lastWord("");

    const auto& updateEntropy = [this, &suffixSet, &lastWord]() {
      if (lastWord.UTF8Length() > 0) {
        suffixEntropies[lastWord] = Entropy(suffixSet);
        suffixSet.clear();
      }
    };

    for (const auto& suffix : suffixes) {
      if (suffix.UTF8Length() < length) {
        continue;
      }
      const auto& wordCandidate = suffix.Left(length);
      if (wordCandidate != lastWord) {
        updateEntropy();
        lastWord = wordCandidate;
      }
      if (length + suffixSetLength <= suffix.UTF8Length()) {
        const auto& wordSuffix = suffix.SubString(length, suffixSetLength);
        suffixSet[wordSuffix]++;
      }
    }
    updateEntropy();
  }
}

void WordDetection::CalculateFrequency() {
  for (const auto& suffix : suffixes) {
    for (size_t i = 1; i <= suffix.UTF8Length() && i <= wordMaxLength; i++) {
      const UTF8StringSlice wordCandidate = suffix.Left(i);
      frequencies[wordCandidate]++;
      totalOccurrence++;
    }
  }
  logTotalOccurrence = log(totalOccurrence);
}

void WordDetection::CalculateCohesions() {
  for (const auto& item : frequencies) {
    const auto& wordCandidate = item.first;
    cohesions[wordCandidate] = Cohesion(wordCandidate);
  }
}

double WordDetection::Probability(const UTF8StringSlice& word) const {
  const auto& frequencyIterator = frequencies.find(word);
  if (frequencyIterator != frequencies.end()) {
    return frequencyIterator->second / static_cast<double>(totalOccurrence);
  } else {
    return 0;
  }
}

double WordDetection::LogProbability(const UTF8StringSlice& word) const {
  // log(frequency / totalOccurrence) = log(frequency) - log(totalOccurrence)
  const auto& frequencyIterator = frequencies.find(word);
  if (frequencyIterator != frequencies.end()) {
    return log(frequencyIterator->second) - logTotalOccurrence;
  } else {
    return -INFINITY;
  }
}

double WordDetection::PMI(const UTF8StringSlice& wordCandidate,
                          const UTF8StringSlice& part1,
                          const UTF8StringSlice& part2) const {
  // PMI(x, y) = log(P(x, y) / (P(x) * P(y)))
  //           = log(P(x, y)) - log(P(x)) - log(P(y))
  return LogProbability(wordCandidate) - LogProbability(part1) -
         LogProbability(part2);
}

double WordDetection::Cohesion(const UTF8StringSlice& wordCandidate) const {
  // TODO Try average value
  double minPMI = INFINITY;
  for (size_t leftLength = 1; leftLength <= wordCandidate.UTF8Length() - 1;
       leftLength++) {
    const auto& leftPart = wordCandidate.Left(leftLength);
    const auto& rightPart =
        wordCandidate.Right(wordCandidate.UTF8Length() - leftLength);
    double pmi = PMI(wordCandidate, leftPart, rightPart);
    minPMI = std::min(pmi, minPMI);
  }
  return minPMI;
}

double
WordDetection::Entropy(const std::map<UTF8StringSlice, size_t>& choices) const {
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

} // namespace tools
} // namespace opencc

int main(int argc, const char* argv[]) {
  opencc::tools::WordDetection wordDetection(3);
  wordDetection.Detect("四是四十是十十四是十四四十是四十");
  return 0;
}
