/*
 * Open Chinese Convert
 *
 * Copyright 2015 BYVoid <byvoid@byvoid.com>
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

#include "DartsDict.hpp"
#include "TextDictTestBase.hpp"

namespace opencc {

class DartsDictTest : public TextDictTestBase {
protected:
  DartsDictTest()
      : dartsDict(DartsDict::NewFromDict(*textDict.get())),
        fileName("dict.ocd"){};

  const DartsDictPtr dartsDict;
  const string fileName;
};

TEST_F(DartsDictTest, DictTest) { TestDict(dartsDict); }

TEST_F(DartsDictTest, Serialization) {
  dartsDict->opencc::SerializableDict::SerializeToFile(fileName);
}

TEST_F(DartsDictTest, Deserialization) {
  const DartsDictPtr& deserialized =
      SerializableDict::NewFromFile<DartsDict>(fileName);
  const LexiconPtr& lex1 = dartsDict->GetLexicon();
  const LexiconPtr& lex2 = deserialized->GetLexicon();

  // Compare every entry
  EXPECT_EQ(lex1->Length(), lex2->Length());
  for (size_t i = 0; i < lex1->Length(); i++) {
    EXPECT_EQ(string(lex1->At(i)->Key()), lex2->At(i)->Key());
    EXPECT_EQ(lex1->At(i)->NumValues(), lex2->At(i)->NumValues());
  }

  const TextDictPtr deserializedTextDict(new TextDict(lex2));
  TestDict(deserializedTextDict);
}

} // namespace opencc
