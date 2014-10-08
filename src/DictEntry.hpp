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

namespace opencc {
class OPENCC_EXPORT DictEntry {
public:
  static DictEntry ParseKeyValues(const char* buff);

  DictEntry() {
  }

  DictEntry(const string& _key) : key(_key) {
  }

  DictEntry(const string& _key, const string& _value)
      : key(_key), values(vector<string>{_value}) {
  }

  DictEntry(const string& _key, const vector<string>& _values)
      : key(_key), values(_values) {
  }

  const string& Key() const {
    return key;
  }

  const vector<string>& Values() const {
    return values;
  }

  const string& GetDefault() const {
    if (values.size() > 0) {
      return values[0];
    } else {
      return key;
    }
  }

  bool operator<(const DictEntry& that) const {
    return key < that.key;
  }

private:
  string key;
  vector<string> values;
};
}
