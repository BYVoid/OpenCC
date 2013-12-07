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

#include "Conversion.hpp"
#include "Dict.hpp"
#include "DictGroup.hpp"
#include "DartsDict.hpp"
#include "TextDict.hpp"
#include "TestUtils.hpp"

namespace Opencc {
  class DictTestUtils {
  public:
    static TextDictPtr CreateTextDictForText() {
      TextDictPtr textDict(new TextDict);
      textDict->AddKeyValue(DictEntry("BYVoid", "byv"));
      textDict->AddKeyValue(DictEntry("zigzagzig", "zag"));
      textDict->AddKeyValue(DictEntry("積羽沉舟", "羣輕折軸"));
      textDict->AddKeyValue(DictEntry("清", "Tsing"));
      textDict->AddKeyValue(DictEntry("清華", "Tsinghua"));
      textDict->AddKeyValue(DictEntry("清華大學", "TsinghuaUniversity"));
      return textDict;
    }
    
    static DictPtr CreateDictForCharacters() {
      TextDictPtr textDict(new TextDict);
      textDict->AddKeyValue(DictEntry("后", StringVector{"后", "後"}));
      textDict->AddKeyValue(DictEntry("发", StringVector{"發", "髮"}));
      textDict->AddKeyValue(DictEntry("干", StringVector{"幹", "乾", "干"}));
      textDict->AddKeyValue(DictEntry("里", StringVector{"裏", "里"}));
      return textDict;
    }
    
    static DictPtr CreateDictForPhrases() {
      TextDictPtr textDict(new TextDict);
      textDict->AddKeyValue(DictEntry("太后", "太后"));
      textDict->AddKeyValue(DictEntry("头发", "頭髮"));
      textDict->AddKeyValue(DictEntry("干燥", "乾燥"));

      DartsDictPtr dartsDict(new DartsDict);
      dartsDict->LoadFromDict(textDict.get());
      return dartsDict;
    }
    
    static DictGroupPtr CreateDictGroupForConversion() {
      DictGroupPtr dictGroup(new DictGroup);
      DictPtr phrasesDict = CreateDictForPhrases();
      DictPtr charactersDict = CreateDictForCharacters();
      dictGroup->AddDict(phrasesDict);
      dictGroup->AddDict(charactersDict);
      return dictGroup;
    }
    
    static DictPtr CreateDictForTaiwanVariants() {
      TextDictPtr textDict(new TextDict);
      textDict->AddKeyValue(DictEntry("裏", "裡"));
      return textDict;
    }
    
    static void TestDict(DictPtr dict) {
      Optional<DictEntryPtr> entry;
      entry = dict->MatchPrefix("BYVoid");
      AssertTrue(!entry.IsNull());
      AssertEquals("BYVoid", entry.Get()->key);
      AssertEquals("byv", entry.Get()->GetDefault());
      
      entry = dict->MatchPrefix("BYVoid123");
      AssertTrue(!entry.IsNull());
      AssertEquals("BYVoid", entry.Get()->key);
      AssertEquals("byv", entry.Get()->GetDefault());
      
      entry = dict->MatchPrefix("積羽沉舟");
      AssertTrue(!entry.IsNull());
      AssertEquals("積羽沉舟", entry.Get()->key);
      AssertEquals("羣輕折軸", entry.Get()->GetDefault());
      
      entry = dict->MatchPrefix("Unknown");
      AssertTrue(entry.IsNull());
      
      DictEntryPtrVectorPtr matches = dict->MatchAllPrefixes("清華大學計算機系");
      AssertEquals(3, matches->size());
      AssertEquals("清華大學", matches->at(0)->key);
      AssertEquals("TsinghuaUniversity", matches->at(0)->GetDefault());
      AssertEquals("清華", matches->at(1)->key);
      AssertEquals("Tsinghua", matches->at(1)->GetDefault());
      AssertEquals("清", matches->at(2)->key);
      AssertEquals("Tsing", matches->at(2)->GetDefault());
    }
  };
}
