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

namespace opencc {
  class OPENCC_EXPORT DictEntry {
    public:
      string key;
      vector<string> values;
      DictEntry() {}

      DictEntry(string key_) : key(key_) {}

      DictEntry(string key_, string value_) : key(key_) {
        values.push_back(value_);
      }

      DictEntry(string key_, vector<string> values_)
        : key(key_), values(values_) {}

      bool operator<(const DictEntry& that) const {
        return key < that.key;
      }

      string GetDefault() const {
        if (values.size() > 0) {
          return values[0];
        } else {
          return key;
        }
      }

      static bool Cmp(DictEntry a, DictEntry b) {
        return a.key < b.key;
      }
  };
}
