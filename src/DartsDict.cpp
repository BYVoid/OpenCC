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

using namespace Opencc;

static const char* OCDHEADER = "OPENCCDARTS1";

DartsDict::DartsDict() : lexicon(new vector<DictEntry>) {
  maxLength = 0;
  buffer = nullptr;
}

DartsDict::~DartsDict() {
  if (buffer != nullptr) {
    free(buffer);
  }
}

size_t DartsDict::KeyMaxLength() const {
  return maxLength;
}

Optional<DictEntry*> DartsDict::MatchPrefix(const char* word) {
  string wordTrunc = UTF8Util::Truncate(word, maxLength);
  // TODO reduce 1 by utf8 char
  for (size_t len = wordTrunc.length(); len > 0; len--) {
    wordTrunc.resize(len);
    Darts::DoubleArray::result_pair_type result;
    dict.exactMatchSearch(wordTrunc.c_str(), result);
    if (result.value != -1) {
      return Optional<DictEntry*>(&lexicon->at(result.value));
    }
  }
  return Optional<DictEntry*>();
}

shared_ptr<vector<DictEntry*>> DartsDict::MatchAllPrefixes(const char* word) {
  shared_ptr<vector<DictEntry*>> matchedLengths;
  matchedLengths.reset(new vector<DictEntry*>);
  string wordTrunc = UTF8Util::Truncate(word, maxLength);
  for (size_t len = wordTrunc.length(); len > 0; len--) {
    wordTrunc.resize(len);
    Darts::DoubleArray::result_pair_type result;
    dict.exactMatchSearch(wordTrunc.c_str(), result);
    if (result.value != -1) {
      matchedLengths->push_back(&lexicon->at(result.value));
    }
  }
  return matchedLengths;
}

shared_ptr<vector<DictEntry>> DartsDict::GetLexicon() {
  return lexicon;
}

void DartsDict::LoadFromDict(Dict& dictionary) {
  vector<const char*> keys;
  maxLength = 0;
  lexicon = dictionary.GetLexicon();
  size_t lexiconCount = lexicon->size();
  keys.resize(lexiconCount);
  for (size_t i = 0; i < lexiconCount; i++) {
    const DictEntry& entry = lexicon->at(i);
    keys[i] = entry.key.c_str();
    maxLength = std::max(entry.key.length(), maxLength);
  }
  dict.build(lexicon->size(), &keys[0]);
}

void DartsDict::SerializeToFile(const string fileName) {
  FILE *fp = fopen(fileName.c_str(), "wb");
  if (fp == NULL) {
    fprintf(stderr, _("Can not write file: %s\n"), fileName.c_str());
    exit(1);
  }
  SerializeToFile(fp);
  fclose(fp);
}

void DartsDict::SerializeToFile(FILE* fp) {
  fwrite(OCDHEADER, sizeof(char), strlen(OCDHEADER), fp);

  size_t dartsSize = dict.total_size();
  fwrite(&dartsSize, sizeof(size_t), 1, fp);
  fwrite(dict.array(), sizeof(char), dartsSize, fp);

  TextDict textDict;
  textDict.LoadFromDict(*this);
  textDict.SerializeToFile(fp);
}

void DartsDict::LoadFromFile(const string fileName) {
  FILE *fp = fopen(fileName.c_str(), "rb");
  if (fp == NULL) {
    // TODO exception
    fprintf(stderr, _("Can not read file: %s\n"), fileName.c_str());
    exit(1);
  }
  LoadFromFile(fp);
  fclose(fp);
}

void DartsDict::LoadFromFile(FILE* fp) {
  if (buffer != nullptr) {
    free(buffer);
  }
  buffer = malloc(sizeof(char) * strlen(OCDHEADER));
  fread(buffer, sizeof(char), strlen(OCDHEADER), fp);
  if (memcmp(buffer, OCDHEADER, strlen(OCDHEADER)) != 0) {
    throw std::runtime_error("Invalid format");
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
