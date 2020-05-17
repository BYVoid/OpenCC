/*
 * Open Chinese Convert
 *
 * Copyright 2010-2020 Carbo Kuo <byvoid@byvoid.com>
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
#include "Segments.hpp"
#include "UTF8Util.hpp"

namespace opencc {
/**
 * Key-values pair entry
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT DictEntry {
public:
  virtual ~DictEntry() {}

  virtual std::string Key() const = 0;

  virtual std::vector<std::string> Values() const = 0;

  virtual std::string GetDefault() const = 0;

  virtual size_t NumValues() const = 0;

  virtual std::string ToString() const = 0;

  size_t KeyLength() const { return Key().length(); }

  bool operator<(const DictEntry& that) const { return Key() < that.Key(); }

  bool operator==(const DictEntry& that) const { return Key() == that.Key(); }

  static bool UPtrLessThan(const std::unique_ptr<DictEntry>& a,
                           const std::unique_ptr<DictEntry>& b) {
    return *a < *b;
  }
};

class OPENCC_EXPORT NoValueDictEntry : public DictEntry {
public:
  NoValueDictEntry(const std::string& _key) : key(_key) {}

  virtual ~NoValueDictEntry() {}

  virtual std::string Key() const { return key; }

  virtual std::vector<std::string> Values() const {
    return std::vector<std::string>();
  }

  virtual std::string GetDefault() const { return key; }

  virtual size_t NumValues() const { return 0; }

  virtual std::string ToString() const { return key; }

private:
  std::string key;
};

class OPENCC_EXPORT SingleValueDictEntry : public DictEntry {
public:
  virtual std::string Value() const = 0;

  virtual std::vector<std::string> Values() const {
    return std::vector<std::string>{Value()};
  }

  virtual std::string GetDefault() const { return Value(); }

  virtual size_t NumValues() const { return 1; }

  virtual std::string ToString() const {
    return std::string(Key()) + "\t" + Value();
  }
};

class OPENCC_EXPORT StrSingleValueDictEntry : public SingleValueDictEntry {
public:
  StrSingleValueDictEntry(const std::string& _key, const std::string& _value)
      : key(_key), value(_value) {}

  virtual ~StrSingleValueDictEntry() {}

  virtual std::string Key() const { return key; }

  virtual std::string Value() const { return value; }

private:
  std::string key;
  std::string value;
};

class OPENCC_EXPORT MultiValueDictEntry : public DictEntry {
public:
  virtual std::string GetDefault() const {
    if (NumValues() > 0) {
      return Values().at(0);
    } else {
      return Key();
    }
  }

  virtual std::string ToString() const;
};

class OPENCC_EXPORT StrMultiValueDictEntry : public MultiValueDictEntry {
public:
  StrMultiValueDictEntry(const std::string& _key,
                         const std::vector<std::string>& _values)
      : key(_key), values(_values) {}

  virtual ~StrMultiValueDictEntry() {}

  virtual std::string Key() const { return key; }

  size_t NumValues() const { return values.size(); }

  std::vector<std::string> Values() const { return values; }

private:
  std::string key;
  std::vector<std::string> values;
};

class OPENCC_EXPORT DictEntryFactory {
public:
  static DictEntry* New(const std::string& key) {
    return new NoValueDictEntry(key);
  }

  static DictEntry* New(const std::string& key, const std::string& value) {
    return new StrSingleValueDictEntry(key, value);
  }

  static DictEntry* New(const std::string& key,
                        const std::vector<std::string>& values) {
    if (values.size() == 0) {
      return New(key);
    } else if (values.size() == 1) {
      return New(key, values.front());
    }
    return new StrMultiValueDictEntry(key, values);
  }

  static DictEntry* New(const DictEntry* entry) {
    if (entry->NumValues() == 0) {
      return new NoValueDictEntry(entry->Key());
    } else if (entry->NumValues() == 1) {
      return new StrSingleValueDictEntry(entry->Key(), entry->Values().front());
    } else {
      return new StrMultiValueDictEntry(entry->Key(), entry->Values());
    }
  }
};
} // namespace opencc
