/*
 * Open Chinese Convert
 *
 * Copyright 2010-2013 BYVoid <byvoid@byvoid.com>
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
#include "UTF8Util.hpp"

using namespace opencc;

StringVectorPtr MaxMatchSegmentation::Segment(const string& text) {
  StringVectorPtr segments(new StringVector);
  StringVectorPtr buffer(new StringVector);
  auto clearBuffer = [&segments, &buffer]() {
                       if (buffer->size() > 0) {
                         segments->push_back(UTF8Util::Join(buffer));
                         buffer->clear();
                       }
                     };
  for (const char* pstr = text.c_str(); *pstr != '\0';) {
    Optional<DictEntryPtr> matched = dict->MatchPrefix(pstr);
    size_t matchedLength;
    if (matched.IsNull()) {
      matchedLength = UTF8Util::NextCharLength(pstr);
      buffer->push_back(UTF8Util::FromSubstr(pstr, matchedLength));
    } else {
      clearBuffer();
      matchedLength = matched.Get()->key.length();
      segments->push_back(matched.Get()->key);
    }
    pstr += matchedLength;
  }
  clearBuffer();
  return segments;
}
