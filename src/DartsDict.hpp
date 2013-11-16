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

#pragma once

#include "Common.hpp"
#include "Dict.hpp"
#include "TextDict.hpp"
#include "darts.hh"

namespace Opencc {
  class DartsDict : public Dict {
  public:
    DartsDict(const string fileName);
    virtual ~DartsDict();
    virtual size_t KeyMaxLength() const;
    virtual size_t MatchPrefix(const char* word) const;
    virtual vector<size_t> GetLengthsOfAllMatches(const char* word) const;
    
    void SerializeToFile(const string fileName);
    void FromTextDict(TextDict& dictionary);
    
  private:
    void BuildDarts();
    
    struct Value {
      string value;
      size_t cursor;
      Value(string value_, size_t cursor_) : value(value_), cursor(cursor_) {
      }
    };
    
    struct Entry {
      string key;
      vector<size_t> valueIndexes;
    };
    
    size_t maxLength;
    vector<Entry> lexicon;
    vector<Value> values;
    Darts::DoubleArray dict;
  };
}
