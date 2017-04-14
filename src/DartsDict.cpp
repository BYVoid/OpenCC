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

#include "BinaryDict.hpp"
#include "DartsDict.hpp"
#include "darts.h"
#include "Lexicon.hpp"

using namespace opencc;

static const char* OCDHEADER = "OPENCCDARTS1";

class DartsDict::DartsInternal {
public:
  BinaryDictPtr binary;
  void* buffer;
  Darts::DoubleArray* doubleArray;

  DartsInternal() : binary(nullptr), buffer(nullptr), doubleArray(nullptr) {}

  ~DartsInternal() {
    if (buffer != nullptr) {
      free(buffer);
    }
    if (doubleArray != nullptr) {
      delete doubleArray;
    }
  }
};

DartsDict::DartsDict() { internal = new DartsInternal; }

DartsDict::~DartsDict() { delete internal; }

size_t DartsDict::KeyMaxLength() const { return maxLength; }

Optional<const DictEntry*> DartsDict::Match(const char* word) const {
  Darts::DoubleArray& dict = *internal->doubleArray;
  Darts::DoubleArray::result_pair_type result;

  dict.exactMatchSearch(word, result);
  if (result.value != -1) {
    return Optional<const DictEntry*>(
        lexicon->At(static_cast<size_t>(result.value)));
  } else {
    return Optional<const DictEntry*>::Null();
  }
}

Optional<const DictEntry*> DartsDict::MatchPrefix(const char* word) const {
  const size_t DEFAULT_NUM_ENTRIES = 64;
  Darts::DoubleArray& dict = *internal->doubleArray;
  Darts::DoubleArray::value_type results[DEFAULT_NUM_ENTRIES];
  Darts::DoubleArray::value_type maxMatchedResult;
  size_t numMatched =
      dict.commonPrefixSearch(word, results, DEFAULT_NUM_ENTRIES);
  if (numMatched == 0) {
    return Optional<const DictEntry*>::Null();
  } else if ((numMatched > 0) && (numMatched < DEFAULT_NUM_ENTRIES)) {
    maxMatchedResult = results[numMatched - 1];
  } else {
    Darts::DoubleArray::value_type* rematchedResults =
        new Darts::DoubleArray::value_type[numMatched];
    numMatched = dict.commonPrefixSearch(word, rematchedResults, numMatched);
    maxMatchedResult = rematchedResults[numMatched - 1];
    delete[] rematchedResults;
  }
  if (maxMatchedResult >= 0) {
    return Optional<const DictEntry*>(
        lexicon->At(static_cast<size_t>(maxMatchedResult)));
  } else {
    return Optional<const DictEntry*>::Null();
  }
}

LexiconPtr DartsDict::GetLexicon() const { return lexicon; }

DartsDictPtr DartsDict::NewFromFile(FILE* fp) {
  DartsDictPtr dict(new DartsDict());

  Darts::DoubleArray* doubleArray = new Darts::DoubleArray();
  size_t headerLen = strlen(OCDHEADER);
  void* buffer = malloc(sizeof(char) * headerLen);
  size_t bytesRead = fread(buffer, sizeof(char), headerLen, fp);
  if (bytesRead != headerLen || memcmp(buffer, OCDHEADER, headerLen) != 0) {
    throw InvalidFormat("Invalid OpenCC dictionary header");
  }
  free(buffer);

  size_t dartsSize;
  bytesRead = fread(&dartsSize, sizeof(size_t), 1, fp);
  if (bytesRead * sizeof(size_t) != sizeof(size_t)) {
    throw InvalidFormat("Invalid OpenCC dictionary header (dartsSize)");
  }
  buffer = malloc(dartsSize);
  bytesRead = fread(buffer, 1, dartsSize, fp);
  if (bytesRead != dartsSize) {
    throw InvalidFormat("Invalid OpenCC dictionary size of darts mismatch");
  }
  doubleArray->set_array(buffer);

  auto internal = dict->internal;
  internal->buffer = buffer;
  internal->binary = BinaryDict::NewFromFile(fp);
  internal->doubleArray = doubleArray;
  dict->lexicon = internal->binary->GetLexicon();
  dict->maxLength = internal->binary->KeyMaxLength();
  return dict;
}

DartsDictPtr DartsDict::NewFromDict(const Dict& thatDict) {
  DartsDictPtr dict(new DartsDict());

  Darts::DoubleArray* doubleArray = new Darts::DoubleArray();
  vector<const char*> keys;
  size_t maxLength = 0;
  const LexiconPtr& lexicon = thatDict.GetLexicon();
  size_t lexiconCount = lexicon->Length();
  keys.resize(lexiconCount);
  for (size_t i = 0; i < lexiconCount; i++) {
    const DictEntry* entry = lexicon->At(i);
    keys[i] = entry->Key();
    maxLength = (std::max)(entry->KeyLength(), maxLength);
  }
  doubleArray->build(lexicon->Length(), &keys[0]);
  dict->lexicon = lexicon;
  dict->maxLength = maxLength;
  auto internal = dict->internal;
  internal->doubleArray = doubleArray;
  return dict;
}

void DartsDict::SerializeToFile(FILE* fp) const {
  Darts::DoubleArray& dict = *internal->doubleArray;

  fwrite(OCDHEADER, sizeof(char), strlen(OCDHEADER), fp);

  size_t dartsSize = dict.total_size();
  fwrite(&dartsSize, sizeof(size_t), 1, fp);
  fwrite(dict.array(), sizeof(char), dartsSize, fp);

  internal->binary.reset(new BinaryDict(lexicon));
  internal->binary->SerializeToFile(fp);
}
