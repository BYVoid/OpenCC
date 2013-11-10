/*
 * Open Chinese Convert
 *
 * Copyright 2013-2013 BYVoid <byvoid@byvoid.com>
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

#ifndef __OPENCC_DICT_STORAGE_H_
#define __OPENCC_DICT_STORAGE_H_

#include "common.h"
#include <string>
#include <vector>
#include "darts.hh"

using std::string;
using std::vector;

namespace Opencc {

  class DictStorage {
  public:
    void serialize(Dict* dictionary, const string fileName);
    
  private:
    void getLexicon(Dict* dictionary);
    void buildDarts();
    void writeToFile(const string fileName);
    
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
    
    vector<Entry> lexicon;
    vector<Value> values;
    Darts::DoubleArray dict;
  };

}

#endif
