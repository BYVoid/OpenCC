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

#include "PhraseExtract.hpp"

using opencc::UTF8StringSlice;
using opencc::PhraseExtract;

int main(int argc, const char* argv[]) {
  std::ifstream ifs("/Users/byvoid/a.txt");
  const string content((std::istreambuf_iterator<char>(ifs)),
                       (std::istreambuf_iterator<char>()));
  PhraseExtract wordDetection(4);
  wordDetection.SetPreCalculationFilter(
      [](const UTF8StringSlice& word) { return word.UTF8Length() < 2; });
  wordDetection.SetPostCalculationFilter(
      [&wordDetection](const UTF8StringSlice& word) {
        const size_t frequency = wordDetection.Frequency(word);
        const double cohesion = wordDetection.Cohesion(word);
        const double entropy = wordDetection.Entropy(word);
        const double suffixEntropy = wordDetection.SuffixEntropy(word);
        const double prefixEntropy = wordDetection.PrefixEntropy(word);
        bool accept = cohesion >= 3.5 && entropy >= 3.3 &&
                      prefixEntropy >= 0.5 && suffixEntropy >= 0.5;
        return !accept;
      });

  wordDetection.Detect(content.c_str());
  for (const auto& word : wordDetection.Words()) {
    const size_t frequency = wordDetection.Frequency(word);
    const double cohesion = wordDetection.Cohesion(word);
    const double suffixEntropy = wordDetection.SuffixEntropy(word);
    const double prefixEntropy = wordDetection.PrefixEntropy(word);
    const double entropy = wordDetection.Entropy(word);
    std::cout << word.ToString() << " " << frequency << " " << cohesion << " "
              << entropy << " " << prefixEntropy << " " << suffixEntropy
              << std::endl;
  }
  return 0;
}
