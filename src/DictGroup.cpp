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

#include "DictGroup.hpp"

using namespace Opencc;

DictGroup::DictGroup() {
}

DictGroup::~DictGroup() {
}

void DictGroup::AddDict(const Dict* dict) {
  dicts.push_back(dict);
}

vector<size_t> DictGroup::GetLengthsOfAllMatches(const char* word) {
  vector<size_t> matchedLengths;
  for (const Dict* dict : dicts) {
    vector<size_t> lengths = dict->GetLengthsOfAllMatches(word);
    for (size_t length : lengths) {
      matchedLengths.push_back(length);
    }
  }
  std::sort(matchedLengths.begin(), matchedLengths.end());
  vector<size_t> matchedLengthsDeduplicated;
  size_t last = 0;
  for (size_t length : matchedLengths) {
    if (length != last) {
      matchedLengthsDeduplicated.push_back(length);
    }
    last = length;
  }
  return matchedLengthsDeduplicated;
}
