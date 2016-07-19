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

#include "Dict.hpp"

using namespace opencc;

Optional<const DictEntry*> Dict::MatchPrefix(const char* word) const {
  string wordTrunc = UTF8Util::TruncateUTF8(word, KeyMaxLength());
  const char* wordTruncPtr = wordTrunc.c_str() + wordTrunc.length();
  for (long len = static_cast<long>(wordTrunc.length()); len > 0;) {
    wordTrunc.resize(static_cast<size_t>(len));
    wordTruncPtr = wordTrunc.c_str() + len;
    const Optional<const DictEntry*>& result = Match(wordTrunc.c_str());
    if (!result.IsNull()) {
      return result;
    }
    len -= static_cast<long>(UTF8Util::PrevCharLength(wordTruncPtr));
  }
  return Optional<const DictEntry*>::Null();
}

vector<const DictEntry*> Dict::MatchAllPrefixes(const char* word) const {
  vector<const DictEntry*> matchedLengths;
  string wordTrunc = UTF8Util::TruncateUTF8(word, KeyMaxLength());
  const char* wordTruncPtr = wordTrunc.c_str() + wordTrunc.length();
  for (long len = static_cast<long>(wordTrunc.length()); len > 0;
       len -= static_cast<long>(UTF8Util::PrevCharLength(wordTruncPtr))) {
    wordTrunc.resize(static_cast<size_t>(len));
    wordTruncPtr = wordTrunc.c_str() + len;
    const Optional<const DictEntry*>& result = Match(wordTrunc.c_str());
    if (!result.IsNull()) {
      matchedLengths.push_back(result.Get());
    }
  }
  return matchedLengths;
}
