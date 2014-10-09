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

namespace opencc {
class OPENCC_EXPORT Segments {
public:
  Segments() {
  }

  Segments(std::initializer_list<const char*> initList) : segments(initList) {
  }

  Segments(std::initializer_list<string> initList) {
    for (const string& item : initList) {
      AddSegment(item);
    }
  }

  void AddSegment(const char* unmanagedString) {
    segments.push_back(unmanagedString);
  }

  void AddSegment(const string& str) {
    memory.push_back(str);
    const char* ptr = memory.back().c_str();
    segments.push_back(ptr);
  }

  const char* At(size_t index) const {
    return segments.at(index);
  }

  size_t Length() const {
    return segments.size();
  }

  vector<const char*>::const_iterator begin() const {
    return segments.begin();
  }

  vector<const char*>::const_iterator end() const {
    return segments.end();
  }

private:
  vector<const char*> segments;
  vector<string> memory;
};
}
