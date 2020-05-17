/*
 * Open Chinese Convert
 *
 * Copyright 2020 Carbo Kuo <byvoid@byvoid.com>
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

#include "SerializedValues.hpp"
#include "TextDictTestBase.hpp"

namespace opencc {

class SerializedValuesTest : public TextDictTestBase {
protected:
  SerializedValuesTest()
      : binDict(new SerializedValues(textDict->GetLexicon())),
        fileName("dict.bin"){};

  const std::shared_ptr<SerializedValues> binDict;
  const std::string fileName;
};

TEST_F(SerializedValuesTest, Serialization) {
  binDict->opencc::SerializableDict::SerializeToFile(fileName);
}

TEST_F(SerializedValuesTest, Deserialization) {
  const std::shared_ptr<SerializedValues>& deserialized =
      SerializableDict::NewFromFile<SerializedValues>(fileName);
  const LexiconPtr& lex1 = binDict->GetLexicon();
  const LexiconPtr& lex2 = deserialized->GetLexicon();

  // Compare every entry
  EXPECT_EQ(lex1->Length(), lex2->Length());
  for (size_t i = 0; i < lex1->Length(); i++) {
    EXPECT_EQ(lex1->At(i)->NumValues(), lex2->At(i)->NumValues());
  }
}

} // namespace opencc
