/*
 * Open Chinese Convert
 *
 * Copyright 2010-2026 Carbo Kuo and contributors
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

class PrefixMatch {
public:
  struct Match {
    bool matched;
    size_t keyLength;
    const std::string* key;
    const std::string* value;
  };

  explicit PrefixMatch(const DictPtr& dict);
  ~PrefixMatch();

  Match MatchPrefix(const char* word, size_t len) const;

private:
  class Table;

  void AddDict(const DictPtr& dict);

  std::vector<std::unique_ptr<Table>> tables;
};

} // namespace opencc
