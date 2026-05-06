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

#include "MaxMatchSegmentation.hpp"
#include "PrefixMatch.hpp"
#include "Segments.hpp"
#include "UTF8Util.hpp"

using namespace opencc;

MaxMatchSegmentation::MaxMatchSegmentation(const DictPtr _dict)
    : dict(_dict), prefixMatch(new PrefixMatch(_dict)) {}

SegmentsPtr MaxMatchSegmentation::Segment(const std::string& text) const {
  SegmentsPtr segments(new Segments);
  const char* segStart = text.c_str();
  size_t segLength = 0;
  auto clearBuffer = [&segments, &segStart, &segLength]() {
    if (segLength > 0) {
      segments->AddSegment(UTF8Util::FromSubstr(segStart, segLength));
      segLength = 0;
    }
  };
  const char* textEnd = text.c_str() + text.length();
  for (const char* pstr = text.c_str(); *pstr != '\0';) {
    size_t remainingLength = textEnd - pstr;
    const PrefixMatch::Match matched =
        prefixMatch->MatchPrefix(pstr, remainingLength);
    size_t matchedLength;
    if (!matched.matched) {
      matchedLength = UTF8Util::NextCharLength(pstr);
      // Ensure we don't advance beyond the string boundary
      if (matchedLength > remainingLength) {
        matchedLength = remainingLength;
      }
      segLength += matchedLength;
    } else {
      clearBuffer();
      matchedLength = matched.keyLength;
      segments->AddSegment(*matched.key);
      segStart = pstr + matchedLength;
    }
    pstr += matchedLength;
  }
  clearBuffer();
  return segments;
}
