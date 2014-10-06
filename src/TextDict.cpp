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

#include "TextDict.hpp"
#include "UTF8Util.hpp"

using namespace opencc;

static DictEntry ParseKeyValues(const char* buff) {
  size_t length;
  const char* pbuff = UTF8Util::FindNextInline(buff, '\t');
  if (UTF8Util::IsLineEndingOrFileEnding(*pbuff)) {
    throw InvalidFormat("Invalid text dictionary");
  }
  length = pbuff - buff;
  const string& key = UTF8Util::FromSubstr(buff, length);
  vector<string> values;
  while (!UTF8Util::IsLineEndingOrFileEnding(*pbuff)) {
    buff = pbuff = UTF8Util::NextChar(pbuff);
    pbuff = UTF8Util::FindNextInline(buff, ' ');
    length = pbuff - buff;
    const string& value = UTF8Util::FromSubstr(buff, length);
    values.push_back(value);
  }
  return DictEntry(key, values);
}

static size_t GetKeyMaxLength(const vector<DictEntry>& lexicon) {
  size_t maxLength = 0;
  for (const auto& entry : lexicon) {
    size_t keyLength = entry.Key().length();
    maxLength = std::max(keyLength, maxLength);
  }
  return maxLength;
}

static vector<DictEntry> ParseLexiconFromFile(FILE* fp) {
  const int ENTRY_BUFF_SIZE = 4096;
  char buff[ENTRY_BUFF_SIZE];
  vector<DictEntry> lexicon;
  UTF8Util::SkipUtf8Bom(fp);
  while (fgets(buff, ENTRY_BUFF_SIZE, fp)) {
    const DictEntry& entry = ParseKeyValues(buff);
    lexicon.push_back(entry);
  }
  return lexicon;
}

TextDict::TextDict(const vector<DictEntry>& _lexicon) : maxLength(GetKeyMaxLength(
                                                                    _lexicon)),
                                                        lexicon(_lexicon) {}
                                                                  
TextDict::~TextDict() {}

TextDictPtr TextDict::NewFromSortedFile(FILE* fp) {
  const vector<DictEntry>& lexicon = ParseLexiconFromFile(fp);
  return TextDictPtr(new TextDict(lexicon));
}

TextDictPtr TextDict::NewFromFile(FILE* fp) {
  vector<DictEntry> lexicon = ParseLexiconFromFile(fp);
  return NewFromUnsorted(lexicon);
}

TextDictPtr TextDict::NewFromDict(const Dict& dict) {
  return TextDictPtr(new TextDict(dict.GetLexicon()));
}

TextDictPtr TextDict::NewFromUnsorted(const vector<DictEntry>& _lexicon) {
  vector<DictEntry> lexicon = _lexicon;
  std::sort(lexicon.begin(), lexicon.end());
  return TextDictPtr(new TextDict(lexicon));
}

size_t TextDict::KeyMaxLength() const {
  return maxLength;
}

Optional<const DictEntry*> TextDict::Match(const char* word) const {
  DictEntry entry(word);
  auto found = std::lower_bound(lexicon.begin(), lexicon.end(), entry);
  if ((found != lexicon.end()) && (found->Key() == entry.Key())) {
    return Optional<const DictEntry*>(&(*found));
  } else {
    return Optional<const DictEntry*>();
  }
}

vector<DictEntry> TextDict::GetLexicon() const {
  return lexicon;
}

void TextDict::SerializeToFile(FILE* fp) const {
  // TODO escape space
  for (const auto& entry : lexicon) {
    fprintf(fp, "%s\t", entry.Key().c_str());
    size_t i = 0;
    for (const auto& value : entry.Values()) {
      fprintf(fp, "%s", value.c_str());
      if (i < entry.Values().size() - 1) {
        fprintf(fp, " ");
      }
      i++;
    }
    fprintf(fp, "\n");
  }
}
