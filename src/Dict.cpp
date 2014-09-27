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

#include "Dict.hpp"
#include "UTF8Util.hpp"

using namespace opencc;

Optional<DictEntryPtr> Dict::MatchPrefix(const char* word) {
  string wordTrunc = UTF8Util::TruncateUTF8(word, KeyMaxLength());
  const char* wordTruncPtr = wordTrunc.c_str() + wordTrunc.length();
  for (long len = wordTrunc.length(); len > 0;
       len -= UTF8Util::PrevCharLength(wordTruncPtr)) {
    wordTrunc.resize(len);
    wordTruncPtr = wordTrunc.c_str() + len;
    Optional<DictEntryPtr> result = Match(wordTrunc.c_str());
    if (!result.IsNull()) {
      return result;
    }
  }
  return Optional<DictEntryPtr>();
}

DictEntryPtrVectorPtr Dict::MatchAllPrefixes(const char* word) {
  DictEntryPtrVectorPtr matchedLengths(new DictEntryPtrVector);
  string wordTrunc = UTF8Util::TruncateUTF8(word, KeyMaxLength());
  const char* wordTruncPtr = wordTrunc.c_str() + wordTrunc.length();
  for (long len = wordTrunc.length(); len > 0;
       len -= UTF8Util::PrevCharLength(wordTruncPtr)) {
    wordTrunc.resize(len);
    wordTruncPtr = wordTrunc.c_str() + len;
    Optional<DictEntryPtr> result = Match(wordTrunc.c_str());
    if (!result.IsNull()) {
      matchedLengths->push_back(result.Get());
    }
  }
  return matchedLengths;
}
