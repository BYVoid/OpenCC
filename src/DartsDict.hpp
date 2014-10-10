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
#include "SerializableDict.hpp"
#include "TextDict.hpp"

namespace opencc {
/**
* Darts dictionary
*/
class OPENCC_EXPORT DartsDict : public SerializableDict {
public:
  DartsDict(const size_t _maxLength,
            const LexiconPtr& _lexicon,
            const void* _doubleArray,
            const void* _buffer);

  virtual ~DartsDict();

  virtual size_t KeyMaxLength() const;

  virtual Optional<const DictEntry*> Match(const char* word) const;

  virtual Optional<const DictEntry*> MatchPrefix(const char* word) const;

  virtual LexiconPtr GetLexicon() const;

  virtual void SerializeToFile(FILE* fp) const;

  /**
  * Constructs a DartsDict from another dictionary.
  */
  static DartsDictPtr NewFromDict(const Dict& dict);

  static DartsDictPtr NewFromFile(FILE* fp);

private:
  const size_t maxLength;
  const LexiconPtr lexicon;
  const void* doubleArray;
  const void* buffer;
};
}
