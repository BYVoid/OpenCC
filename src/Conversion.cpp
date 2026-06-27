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

#include "Conversion.hpp"
#include "PrefixMatch.hpp"
#include "Segments.hpp"
#include "UTF8Util.hpp"

using namespace opencc;

Conversion::Conversion(DictPtr _dict)
    : dict(_dict), prefixMatch(new PrefixMatch(_dict)) {}

void Conversion::AppendConverted(std::string_view phrase,
                                  std::string* output) const {
  if (phrase.empty()) {
    return;
  }
  const size_t phraseLength = phrase.size();
  output->reserve(output->size() + phraseLength + phraseLength / 5);

  const char* phraseData = phrase.data();
  const char* phraseEnd = phraseData + phraseLength;
  for (const char* pstr = phraseData; pstr < phraseEnd;) {
    size_t remainingLength = phraseEnd - pstr;
    const PrefixMatchView matched =
        prefixMatch->MatchPrefixView(pstr, remainingLength);
    size_t matchedLength;
    if (!matched.matched) {
      matchedLength =
          UTF8Util::NextIdeographicDescriptionSequenceLength(pstr,
                                                             remainingLength);
      if (matchedLength == 0) {
        matchedLength = UTF8Util::NextCharLength(pstr);
      }
      if (matchedLength > remainingLength) {
        matchedLength = remainingLength;
      }
      output->append(pstr, matchedLength);
    } else {
      matchedLength = matched.keyLength;
      if (matchedLength > remainingLength) {
        matchedLength = remainingLength;
      }
      output->append(matched.value.data(), matched.value.size());
    }
    pstr += matchedLength;
  }
}

std::string Conversion::Convert(std::string_view phrase) const {
  if (phrase.empty()) {
    return std::string();
  }
  std::string buffer;
  AppendConverted(phrase, &buffer);
  return buffer;
}

std::string Conversion::Convert(const char* phrase) const {
  return Convert(std::string_view(phrase));
}

void Conversion::AppendConverted(const char* phrase, std::string* output) const {
  AppendConverted(std::string_view(phrase), output);
}

SegmentsPtr Conversion::Convert(const SegmentsPtr& input) const {
  SegmentsPtr output(new Segments);
  for (const char* segment : *input) {
    output->AddSegment(Convert(segment));
  }
  return output;
}
