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

#include "UTF8Util.hpp"
#include "TextDict.hpp"

using namespace Opencc;

#define ENTRY_BUFF_SIZE 128

TextDict::TextEntry parseKeyValues(const char* buff) {
  size_t length;
  const char* pbuff = UTF8Util::findNextInline(buff, '\t');
  if (UTF8Util::isLineEndingOrFileEnding(*pbuff)) {
    throw runtime_error("invalid format");
  }
  length = pbuff - buff;
  // TODO copy
  TextDict::TextEntry entry(UTF8Util::fromSubstr(buff, length));
  while (!UTF8Util::isLineEndingOrFileEnding(*pbuff)) {
    buff = pbuff = UTF8Util::nextChar(pbuff);
    pbuff = UTF8Util::findNextInline(buff, ' ');
    length = pbuff - buff;
    // TODO copy
    string value = UTF8Util::fromSubstr(buff, length);
    entry.values.push_back(value);
  }
  return entry;
}

TextDict::TextDict(const string fileName) {
  // TODO use dynamic getline
  static char buff[ENTRY_BUFF_SIZE];

  FILE* fp = fopen(fileName.c_str(), "r");
  if (fp == NULL) {
    throw runtime_error("file not found");
  }
  UTF8Util::skipUtf8Bom(fp);

  while (fgets(buff, ENTRY_BUFF_SIZE, fp)) {
    // TODO reduce object copies
    TextEntry entry = parseKeyValues(buff);
    lexicon.push_back(entry);
    size_t keyLength = entry.key.length();
    maxLength = std::max(keyLength, maxLength);
  }

  fclose(fp);
  std::sort(lexicon.begin(), lexicon.end());
}


TextDict::~TextDict() {
}

vector<size_t> TextDict::getLengthsOfAllMatches(const char* word) const {
  // TODO copy
  vector<size_t> matchedLengths;
  TextDict::TextEntry entry(word);
  for (size_t len = entry.key.length(); len > 0; len--) {
    entry.key[len] = '\0';
    bool found = std::binary_search(lexicon.begin(), lexicon.end(), entry);
    if (found) {
      matchedLengths.push_back(len);
    }
  }
  return matchedLengths;
}

vector<TextDict::TextEntry> TextDict::getLexicon() const {
  // TODO copy
  return lexicon;
}
