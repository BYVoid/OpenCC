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

#pragma once

#include "Common.hpp"
#include "UTF8Util.hpp"
#include "Segments.hpp"

namespace opencc {
class OPENCC_EXPORT DictEntry {
public:
  static DictEntry ParseKeyValues(const char* buff);

  DictEntry() {
  }

  DictEntry(const string& _key) : key(_key) {
  }

  DictEntry(const string& _key, const string& _value)
      : key(_key), values(Segments{_value}) {
  }

  DictEntry(const string& _key, const Segments& _values)
      : key(_key), values(_values) {
  }

  const char* Key() const {
    return key.c_str();
  }

  size_t KeyLength() const {
    return key.length();
  }

  const Segments& Values() const {
    return values;
  }

  const char* GetDefault() const {
    if (values.Length() > 0) {
      return values.At(0);
    } else {
      return key.c_str();
    }
  }

  bool operator<(const DictEntry& that) const {
    return key < that.key;
  }
  
  bool operator==(const DictEntry& that) const {
    return key == that.key;
  }

private:
  string key;
  Segments values;
};
}
