/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 BYVoid <byvoid@byvoid.com>
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

using namespace opencc;

SegmentsPtr MaxMatchSegmentation::Segment(const string& text) const {
  SegmentsPtr segments(new Segments);
  const char* segStart = text.c_str();
  size_t segLength = 0;
  auto clearBuffer = [&segments, &segStart, &segLength]() {
    if (segLength > 0) {
      segments->AddSegment(UTF8Util::FromSubstr(segStart, segLength));
      segLength = 0;
    }
  };
  for (const char* pstr = text.c_str(); *pstr != '\0';) {
    const Optional<const DictEntry*>& matched = dict->MatchPrefix(pstr);
    size_t matchedLength;
    if (matched.IsNull()) {
      matchedLength = UTF8Util::NextCharLength(pstr);
      segLength += matchedLength;
    } else {
      clearBuffer();
      matchedLength = matched.Get()->KeyLength();
      segments->AddSegment(matched.Get()->Key());
      segStart = pstr + matchedLength;
    }
    pstr += matchedLength;
  }
  clearBuffer();
  return segments;
}
