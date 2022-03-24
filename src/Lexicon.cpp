/*
 * Open Chinese Convert
 *
 * Copyright 2020 Carbo Kuo <byvoid@byvoid.com>
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

#include <algorithm>

#include "Lexicon.hpp"
namespace opencc {

void Lexicon::Sort() {
  std::sort(entries.begin(), entries.end(), DictEntry::UPtrLessThan);
}

bool Lexicon::IsSorted() {
  return std::is_sorted(entries.begin(), entries.end(),
                        DictEntry::UPtrLessThan);
}

bool Lexicon::IsUnique(std::string* dupkey) {
  for (size_t i = 1; i < entries.size(); ++i) {
    if (entries[i - 1]->Key() == entries[i]->Key()) {
      if (dupkey) {
        *dupkey = entries[i]->Key();
      }
      return false;
    }
  }
  return true;
}

} // namespace opencc
