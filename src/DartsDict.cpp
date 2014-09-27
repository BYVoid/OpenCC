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

#include "DartsDict.hpp"
#include "UTF8Util.hpp"
#include "darts.h"

using namespace opencc;

static const char* OCDHEADER = "OPENCCDARTS1";

DartsDict::DartsDict() : lexicon(new DictEntryPtrVector) {
  maxLength = 0;
  buffer = nullptr;
  dict = new Darts::DoubleArray();
}

DartsDict::~DartsDict() {
  if (buffer != nullptr) {
    free(buffer);
  }
  delete (Darts::DoubleArray*)dict;
}

size_t DartsDict::KeyMaxLength() const {
  return maxLength;
}

Optional<DictEntryPtr> DartsDict::Match(const char* word) {
  Darts::DoubleArray& dict = *(Darts::DoubleArray*)this->dict;
  Darts::DoubleArray::result_pair_type result;

  dict.exactMatchSearch(word, result);
  if (result.value != -1) {
    return Optional<DictEntryPtr>(lexicon->at(result.value));
  } else {
    return Optional<DictEntryPtr>();
  }
}

Optional<DictEntryPtr> DartsDict::MatchPrefix(const char* word) {
  const size_t DEFAULT_NUM_ENTRIES = 64;
  Darts::DoubleArray& dict = *(Darts::DoubleArray*)this->dict;
  Darts::DoubleArray::value_type results[DEFAULT_NUM_ENTRIES];
  Darts::DoubleArray::value_type maxMatchedResult = -1;
  size_t numMatched = dict.commonPrefixSearch(word, results, DEFAULT_NUM_ENTRIES);
  if (numMatched == 0) {
    return Optional<DictEntryPtr>();
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
    return Optional<DictEntryPtr>(lexicon->at(maxMatchedResult));
  } else {
    return Optional<DictEntryPtr>();
  }
}

DictEntryPtrVectorPtr DartsDict::GetLexicon() {
  return lexicon;
}

void DartsDict::LoadFromDict(Dict* dictionary) {
  Darts::DoubleArray& dict = *(Darts::DoubleArray*)this->dict;
  std::vector<const char*> keys;

  maxLength = 0;
  lexicon = dictionary->GetLexicon();
  size_t lexiconCount = lexicon->size();
  keys.resize(lexiconCount);
  for (size_t i = 0; i < lexiconCount; i++) {
    DictEntryPtr entry = lexicon->at(i);
    keys[i] = entry->key.c_str();
    maxLength = std::max(entry->key.length(), maxLength);
  }
  dict.build(lexicon->size(), &keys[0]);
}

void DartsDict::SerializeToFile(FILE* fp) {
  Darts::DoubleArray& dict = *(Darts::DoubleArray*)this->dict;

  fwrite(OCDHEADER, sizeof(char), strlen(OCDHEADER), fp);

  size_t dartsSize = dict.total_size();
  fwrite(&dartsSize, sizeof(size_t), 1, fp);
  fwrite(dict.array(), sizeof(char), dartsSize, fp);

  TextDict textDict;
  textDict.LoadFromDict(this);
  textDict.SerializeToFile(fp);
}

void DartsDict::LoadFromFile(FILE* fp) {
  Darts::DoubleArray& dict = *(Darts::DoubleArray*)this->dict;
  if (buffer != nullptr) {
    free(buffer);
  }
  buffer = malloc(sizeof(char) * strlen(OCDHEADER));
  fread(buffer, sizeof(char), strlen(OCDHEADER), fp);
  if (memcmp(buffer, OCDHEADER, strlen(OCDHEADER)) != 0) {
    throw InvalidFormat("Invalid OpenCC dictionary");
  }
  free(buffer);

  size_t dartsSize;
  fread(&dartsSize, sizeof(size_t), 1, fp);
  buffer = malloc(dartsSize);
  fread(buffer, 1, dartsSize, fp);
  dict.set_array(buffer);

  TextDict textDict;
  textDict.LoadFromFile(fp);
  lexicon = textDict.GetLexicon();
  maxLength = textDict.KeyMaxLength();
}
