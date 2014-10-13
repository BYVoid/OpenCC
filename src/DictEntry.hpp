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
  virtual ~DictEntry() {
  }

  virtual const char* Key() const = 0;

  virtual const char* GetDefault() const = 0;

  virtual size_t NumValues() const = 0;

  virtual string ToString() const = 0;

  size_t KeyLength() const {
    return strlen(Key());
  }

  bool operator<(const DictEntry& that) const {
    return strcmp(Key(), that.Key()) < 0;
  }

  bool operator==(const DictEntry& that) const {
    return strcmp(Key(), that.Key()) == 0;
  }

  static bool PtrLessThan(const DictEntry* a, const DictEntry* b) {
    return *a < *b;
  }
};

class OPENCC_EXPORT NoValueDictEntry : public DictEntry {
public:
  NoValueDictEntry(const string& _key) : key(_key) {
  }

  virtual ~NoValueDictEntry() {
  }

  virtual const char* Key() const {
    return key.c_str();
  }

  virtual const char* GetDefault() const {
    return Key();
  }

  virtual size_t NumValues() const {
    return 0;
  }

  virtual string ToString() const {
    return key;
  }

private:
  string key;
};

class OPENCC_EXPORT SingleValueDictEntry : public DictEntry {
public:
  SingleValueDictEntry(const string& _key, const string& _value)
      : key(_key), value(_value) {
  }

  virtual ~SingleValueDictEntry() {
  }

  virtual const char* Key() const {
    return key.c_str();
  }

  virtual const char* Value() const {
    return value.c_str();
  }

  virtual const char* GetDefault() const {
    return value.c_str();
  }

  virtual size_t NumValues() const {
    return 1;
  }

  virtual string ToString() const {
    return key + "\t" + value;
  }

private:
  string key;
  string value;
};

class OPENCC_EXPORT MultiValueDictEntry : public DictEntry {
public:
  MultiValueDictEntry(const string& _key, const Segments& _values)
      : key(_key), values(_values) {
  }

  virtual ~MultiValueDictEntry() {
  }

  virtual const char* Key() const {
    return key.c_str();
  }

  virtual const char* GetDefault() const {
    if (values.Length() > 0) {
      return values.At(0);
    } else {
      return Key();
    }
  }

  virtual size_t NumValues() const;

  virtual string ToString() const;

  const Segments& Values() const {
    return values;
  }

private:
  string key;
  Segments values;
};

class OPENCC_EXPORT DictEntryFactory {
public:
  static DictEntry* New(const string& key) {
    return new NoValueDictEntry(key);
  }

  static DictEntry* New(const string& key, const string& value) {
    return new SingleValueDictEntry(key, value);
  }

  static DictEntry* New(const string& key, const Segments& values) {
    return new MultiValueDictEntry(key, values);
  }

  static DictEntry* New(const DictEntry* entry) {
    if (entry->NumValues() == 0) {
      return new NoValueDictEntry(
          *static_cast<const NoValueDictEntry*>(entry));
    } else if (entry->NumValues() == 1) {
      return new SingleValueDictEntry(
          *static_cast<const SingleValueDictEntry*>(entry));
    } else {
      return new MultiValueDictEntry(
          *static_cast<const MultiValueDictEntry*>(entry));
    }
  }
};

}
