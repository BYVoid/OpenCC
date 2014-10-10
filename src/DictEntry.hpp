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
  DictEntry() : value(Optional<string>::Null()) {
  }

  DictEntry(const string& _key) : key(_key), value(Optional<string>::Null()) {
  }

  DictEntry(const string& _key, const string& _value)
      : key(_key), value(_value) {
  }

  virtual ~DictEntry() {
  }

  const char* Key() const {
    return key.c_str();
  }

  size_t KeyLength() const {
    return key.length();
  }

  virtual size_t NumValues() const {
    return value.IsNull() ? 0 : 1;
  }

  virtual const char* GetDefault() const {
    if (value.IsNull()) {
      return key.c_str();
    } else {
      return value.Get().c_str();
    }
  }

  virtual string ToString() const {
    if (value.IsNull()) {
      return key;
    } else {
      return key + "\t" + value.Get();
    }
  }

  bool operator<(const DictEntry& that) const {
    return key < that.key;
  }
  
  bool operator==(const DictEntry& that) const {
    return key == that.key;
  }

  static bool PtrLessThan(const DictEntry* a, const DictEntry* b) {
    return *a < *b;
  }

private:
  // Disalow copy from subclass
  DictEntry(const MultiValueDictEntry& that): value(Optional<string>::Null()) {
  }

  string key;
  Optional<string> value;
};

class OPENCC_EXPORT MultiValueDictEntry : public DictEntry {
public:
  MultiValueDictEntry(const string& _key, const Segments& _values)
      : DictEntry(_key), values(_values) {
  }

  virtual ~MultiValueDictEntry() {
  }

  const Segments& Values() const {
    return values;
  }

  virtual size_t NumValues() const {
    return values.Length();
  }

  virtual const char* GetDefault() const {
    if (values.Length() > 0) {
      return values.At(0);
    } else {
      return Key();
    }
  }

  virtual string ToString() const {
    // TODO escape space
    size_t i = 0;
    size_t length = values.Length();
    std::ostringstream buffer;
    for (const char* value : values) {
      buffer << value;
      if (i < length - 1) {
        buffer << ' ';
      }
      i++;
    }
    return buffer.str();
  }
private:
  Segments values;
};

}
