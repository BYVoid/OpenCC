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

#include "DartsDict.hpp"
#include "darts.h"

using namespace opencc;

static const char* OCDHEADER = "OPENCCDARTS1";

DartsDict::DartsDict(const size_t _maxLength,
                     const vector<DictEntry>& _lexicon,
                     const void* _doubleArray,
                     const void* _buffer)
    : maxLength(_maxLength), lexicon(_lexicon), doubleArray(_doubleArray), buffer(
    _buffer) {
}

DartsDict::~DartsDict() {
  if (buffer != nullptr) {
    free((void*)buffer);
  }
  delete (Darts::DoubleArray*)doubleArray;
}

size_t DartsDict::KeyMaxLength() const {
  return maxLength;
}

Optional<const DictEntry*> DartsDict::Match(const char* word) const {
  Darts::DoubleArray& dict = *(Darts::DoubleArray*)this->doubleArray;
  Darts::DoubleArray::result_pair_type result;

  dict.exactMatchSearch(word, result);
  if (result.value != -1) {
    return Optional<const DictEntry*>(&lexicon.at(result.value));
  } else {
    return Optional<const DictEntry*>::Null();
  }
}

Optional<const DictEntry*> DartsDict::MatchPrefix(const char* word) const {
  const size_t DEFAULT_NUM_ENTRIES = 64;
  Darts::DoubleArray& dict = *(Darts::DoubleArray*)this->doubleArray;
  Darts::DoubleArray::value_type results[DEFAULT_NUM_ENTRIES];
  Darts::DoubleArray::value_type maxMatchedResult = -1;
  size_t numMatched = dict.commonPrefixSearch(word, results, DEFAULT_NUM_ENTRIES);
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
    return Optional<const DictEntry*>(&lexicon.at(maxMatchedResult));
  } else {
    return Optional<const DictEntry*>::Null();
  }
}

vector<DictEntry> DartsDict::GetLexicon() const {
  return lexicon;
}

DartsDictPtr DartsDict::NewFromFile(FILE* fp) {
  Darts::DoubleArray* doubleArray = new Darts::DoubleArray();

  void* buffer = malloc(sizeof(char) * strlen(OCDHEADER));
  fread(buffer, sizeof(char), strlen(OCDHEADER), fp);
  if (memcmp(buffer, OCDHEADER, strlen(OCDHEADER)) != 0) {
    throw InvalidFormat("Invalid OpenCC dictionary");
  }
  free(buffer);

  size_t dartsSize;
  fread(&dartsSize, sizeof(size_t), 1, fp);
  buffer = malloc(dartsSize);
  fread(buffer, 1, dartsSize, fp);
  doubleArray->set_array(buffer);

  TextDictPtr textDict = TextDict::NewFromSortedFile(fp);
  const vector<DictEntry>& lexicon = textDict->GetLexicon();
  const size_t maxLength = textDict->KeyMaxLength();
  return DartsDictPtr(new DartsDict(maxLength, lexicon, doubleArray, buffer));
}

DartsDictPtr DartsDict::NewFromDict(const Dict& dict) {
  Darts::DoubleArray* doubleArray = new Darts::DoubleArray();
  vector<const char*> keys;
  size_t maxLength = 0;
  const vector<DictEntry>& lexicon = dict.GetLexicon();
  size_t lexiconCount = lexicon.size();
  keys.resize(lexiconCount);
  for (size_t i = 0; i < lexiconCount; i++) {
    const DictEntry& entry = lexicon.at(i);
    keys[i] = entry.Key().c_str();
    maxLength = std::max(entry.Key().length(), maxLength);
  }
  doubleArray->build(lexicon.size(), &keys[0]);
  return DartsDictPtr(new DartsDict(maxLength, lexicon, doubleArray, nullptr));
}

void DartsDict::SerializeToFile(FILE* fp) const {
  Darts::DoubleArray& dict = *(Darts::DoubleArray*)this->doubleArray;

  fwrite(OCDHEADER, sizeof(char), strlen(OCDHEADER), fp);

  size_t dartsSize = dict.total_size();
  fwrite(&dartsSize, sizeof(size_t), 1, fp);
  fwrite(dict.array(), sizeof(char), dartsSize, fp);

  TextDictPtr textDict = TextDict::NewFromDict(*this);
  textDict->SerializeToFile(fp);
}
