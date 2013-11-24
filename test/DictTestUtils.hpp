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

#include "Dict.hpp"
#include "DartsDict.hpp"
#include "TextDict.hpp"

namespace Opencc {
  class DictTestUtils {
  public:
    static TextDict CreateTextDictForText() {
      TextDict textDict;
      textDict.AddKeyValue(DictEntry("BYVoid", "byv"));
      textDict.AddKeyValue(DictEntry("zigzagzig", "zag"));
      textDict.AddKeyValue(DictEntry("積羽沉舟", "羣輕折軸"));
      return textDict;
    }
    
    static void TestDict(Dict& dict) {
      Optional<DictEntry> entry;
      entry = dict.MatchPrefix("BYVoid");
      assert(!entry.IsNull());
      assert(entry.Get().GetDefault() == "byv");
      
      entry = dict.MatchPrefix("BYVoid123");
      assert(!entry.IsNull());
      assert(entry.Get().GetDefault() == "byv");
      
      entry = dict.MatchPrefix("積羽沉舟");
      assert(!entry.IsNull());
      assert(entry.Get().GetDefault() == "羣輕折軸");
      
      entry = dict.MatchPrefix("Unknown");
      assert(entry.IsNull());
    }
  };
}
