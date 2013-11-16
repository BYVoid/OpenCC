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

#ifndef __OPENCC_DICTIONARY_TEXT_H_
#define __OPENCC_DICTIONARY_TEXT_H_

#include "Common.hpp"
#include "Dict.hpp"

namespace Opencc {
  class TextDictionary : public Dict {
  public:
    struct TextEntry {
      string key;
      vector<string> values;
      TextEntry(string key_) : key(key_) {}
      bool operator < (const TextEntry& that) const {
        return key < that.key;
      }
    };
    
    TextDictionary(const string fileName);
    ~TextDictionary();
    vector<size_t> getLengthsOfAllMatches(const char* word) const;
    vector<TextEntry> getLexicon() const;
  private:
    size_t maxLength;
    vector<TextEntry> lexicon;
  };
}

#endif /* __OPENCC_DICTIONARY_TEXT_H_ */
