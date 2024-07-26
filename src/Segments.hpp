/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 Carbo Kuo <byvoid@byvoid.com>
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

#include <sstream>

#include "Common.hpp"

namespace opencc {
/**
 * Segmented text
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT Segments {
public:
  Segments() {}

  Segments(std::initializer_list<const char*> initList) {
    for (const char* item : initList) {
      AddSegment(item);
    }
  }

  Segments(std::initializer_list<std::string> initList) {
    for (const std::string& item : initList) {
      AddSegment(item);
    }
  }

  void AddSegment(const char* unmanagedString) {
    indexes.push_back(std::make_pair(unmanaged.size(), false));
    unmanaged.push_back(unmanagedString);
  }

  void AddSegment(const std::string& str) {
    indexes.push_back(std::make_pair(managed.size(), true));
    managed.push_back(str);
  }

  class iterator : public std::iterator<std::input_iterator_tag, const char*> {
  public:
    iterator(const Segments* const _segments, size_t _cursor)
        : segments(_segments), cursor(_cursor) {}

    iterator& operator++() {
      cursor++;
      return *this;
    }

    bool operator==(const iterator& that) const {
      return cursor == that.cursor && segments == that.segments;
    }

    bool operator!=(const iterator& that) const {
      return !this->operator==(that);
    }

    const char* operator*() const { return segments->At(cursor); }

  private:
    const Segments* const segments;
    size_t cursor;
  };

  const char* At(size_t cursor) const {
    const auto& index = indexes[cursor];
    if (index.second) {
      return managed[index.first].c_str();
    } else {
      return unmanaged[index.first];
    }
  }

  size_t Length() const { return indexes.size(); }

  iterator begin() const { return iterator(this, 0); }

  iterator end() const { return iterator(this, indexes.size()); }

  std::string ToString() const {
    // TODO implement a nested structure to reduce concatenation,
    // like a purely functional differential list
    std::ostringstream buffer;
    for (const char* segment : *this) {
      buffer << segment;
    }
    return buffer.str();
  }

private:
  Segments(const Segments&) {}

  std::vector<const char*> unmanaged;
  std::vector<std::string> managed;
  // index, managed
  std::vector<std::pair<size_t, bool>> indexes;
};
} // namespace opencc
