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
/**
* Key-values pair entry
* @ingroup opencc_cpp_api
*/
class OPENCC_EXPORT DictEntry {
public:
  virtual ~DictEntry() {}

  virtual const char* Key() const = 0;

  virtual vector<const char*> Values() const = 0;

  virtual const char* GetDefault() const = 0;

  virtual size_t NumValues() const = 0;

  virtual string ToString() const = 0;

  size_t KeyLength() const { return strlen(Key()); }

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
  NoValueDictEntry(const string& _key) : key(_key) {}

  virtual ~NoValueDictEntry() {}

  virtual const char* Key() const { return key.c_str(); }

  virtual vector<const char*> Values() const { return vector<const char*>(); }

  virtual const char* GetDefault() const { return Key(); }

  virtual size_t NumValues() const { return 0; }

  virtual string ToString() const { return key; }

private:
  string key;
};

class OPENCC_EXPORT SingleValueDictEntry : public DictEntry {
public:
  virtual const char* Value() const = 0;

  virtual vector<const char*> Values() const {
    return vector<const char*>{Value()};
  }

  virtual const char* GetDefault() const { return Value(); }

  virtual size_t NumValues() const { return 1; }

  virtual string ToString() const { return string(Key()) + "\t" + Value(); }
};

class OPENCC_EXPORT StrSingleValueDictEntry : public SingleValueDictEntry {
public:
  StrSingleValueDictEntry(const string& _key, const string& _value)
      : key(_key), value(_value) {}

  virtual ~StrSingleValueDictEntry() {}

  virtual const char* Key() const { return key.c_str(); }

  virtual const char* Value() const { return value.c_str(); }

private:
  string key;
  string value;
};

class OPENCC_EXPORT MultiValueDictEntry : public DictEntry {
public:
  virtual const char* GetDefault() const {
    if (NumValues() > 0) {
      return Values().at(0);
    } else {
      return Key();
    }
  }

  virtual string ToString() const;
};

class OPENCC_EXPORT StrMultiValueDictEntry : public MultiValueDictEntry {
public:
  StrMultiValueDictEntry(const string& _key, const vector<string>& _values)
      : key(_key), values(_values) {}

  StrMultiValueDictEntry(const string& _key, const vector<const char*>& _values)
      : key(_key) {
    values.reserve(_values.size());
    for (const char* str : _values) {
      values.push_back(str);
    }
  }

  virtual ~StrMultiValueDictEntry() {}

  virtual const char* Key() const { return key.c_str(); }

  size_t NumValues() const { return values.size(); }

  vector<const char*> Values() const {
    vector<const char*> retsult;
    for (const string& value : this->values) {
      retsult.push_back(value.c_str());
    }
    return retsult;
  }

private:
  string key;
  vector<string> values;
};

class OPENCC_EXPORT PtrDictEntry : public MultiValueDictEntry {
public:
  PtrDictEntry(const char* _key, const vector<const char*>& _values)
      : key(_key), values(_values) {}

  virtual ~PtrDictEntry() {}

  virtual const char* Key() const { return key; }

  size_t NumValues() const { return values.size(); }

  vector<const char*> Values() const { return values; }

private:
  const char* key;
  vector<const char*> values;
};

class OPENCC_EXPORT DictEntryFactory {
public:
  static DictEntry* New(const string& key) { return new NoValueDictEntry(key); }

  static DictEntry* New(const string& key, const string& value) {
    return new StrSingleValueDictEntry(key, value);
  }

  static DictEntry* New(const string& key, const vector<string>& values) {
    return new StrMultiValueDictEntry(key, values);
  }

  static DictEntry* New(const DictEntry* entry) {
    if (entry->NumValues() == 0) {
      return new NoValueDictEntry(entry->Key());
    } else if (entry->NumValues() == 1) {
      const auto svEntry = static_cast<const SingleValueDictEntry*>(entry);
      return new StrSingleValueDictEntry(svEntry->Key(), svEntry->Value());
    } else {
      const auto mvEntry = static_cast<const MultiValueDictEntry*>(entry);
      return new StrMultiValueDictEntry(mvEntry->Key(), mvEntry->Values());
    }
  }
};
}
