/*
 * Open Chinese Convert
 *
 * Copyright 2010-2020 Carbo Kuo <byvoid@byvoid.com>
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

#include <algorithm>
#include <cassert>

#include "Lexicon.hpp"
#include "TextDict.hpp"

using namespace opencc;

static size_t GetKeyMaxLength(const LexiconPtr& lexicon) {
  size_t maxLength = 0;
  for (const auto& entry : *lexicon) {
    size_t keyLength = entry->KeyLength();
    maxLength = (std::max)(keyLength, maxLength);
  }
  return maxLength;
}

static DictEntry* ParseKeyValues(const char* buff, size_t lineNum) {
  size_t length;
  if (buff == nullptr || UTF8Util::IsLineEndingOrFileEnding(*buff)) {
    return nullptr;
  }
  const char* pbuff = UTF8Util::FindNextInline(buff, '\t');
  if (UTF8Util::IsLineEndingOrFileEnding(*pbuff)) {
    throw InvalidTextDictionary("Tabular not found " + std::string(buff),
                                lineNum);
  }
  length = static_cast<size_t>(pbuff - buff);
  std::string key = UTF8Util::FromSubstr(buff, length);
  std::vector<std::string> values;
  while (!UTF8Util::IsLineEndingOrFileEnding(*pbuff)) {
    buff = pbuff = UTF8Util::NextChar(pbuff);
    pbuff = UTF8Util::FindNextInline(buff, ' ');
    length = static_cast<size_t>(pbuff - buff);
    const std::string& value = UTF8Util::FromSubstr(buff, length);
    values.push_back(value);
  }
  if (values.size() == 0) {
    throw InvalidTextDictionary("No value in an item", lineNum);
  } else if (values.size() == 1) {
    return DictEntryFactory::New(key, values.at(0));
  } else {
    return DictEntryFactory::New(key, values);
  }
}

static LexiconPtr ParseLexiconFromFile(FILE* fp) {
  const int ENTRY_BUFF_SIZE = 4096;
  char buff[ENTRY_BUFF_SIZE];
  LexiconPtr lexicon(new Lexicon);
  UTF8Util::SkipUtf8Bom(fp);
  size_t lineNum = 1;
  while (fgets(buff, ENTRY_BUFF_SIZE, fp)) {
    DictEntry* entry = ParseKeyValues(buff, lineNum);
    if (entry != nullptr) {
      lexicon->Add(entry);
    }
    lineNum++;
  }
  return lexicon;
}

TextDict::TextDict(const LexiconPtr& _lexicon)
    : maxLength(GetKeyMaxLength(_lexicon)), lexicon(_lexicon) {
  assert(lexicon->IsSorted());
  assert(lexicon->IsUnique());
}

TextDict::~TextDict() {}

TextDictPtr TextDict::NewFromSortedFile(FILE* fp) {
  const LexiconPtr& lexicon = ParseLexiconFromFile(fp);
  return TextDictPtr(new TextDict(lexicon));
}

TextDictPtr TextDict::NewFromFile(FILE* fp) {
  const LexiconPtr& lexicon = ParseLexiconFromFile(fp);
  lexicon->Sort();
  std::string dupkey;
  if (!lexicon->IsUnique(&dupkey)) {
    throw InvalidFormat(
        "The text dictionary contains duplicated keys: " + dupkey + ".");
  }
  return TextDictPtr(new TextDict(lexicon));
}

TextDictPtr TextDict::NewFromDict(const Dict& dict) {
  return TextDictPtr(new TextDict(dict.GetLexicon()));
}

size_t TextDict::KeyMaxLength() const { return maxLength; }

Optional<const DictEntry*> TextDict::Match(const char* word, size_t len) const {
  std::unique_ptr<DictEntry> entry(
      new NoValueDictEntry(std::string(word, len)));
  const auto& found = std::lower_bound(lexicon->begin(), lexicon->end(), entry,
                                       DictEntry::UPtrLessThan);
  if ((found != lexicon->end()) && ((*found)->Key() == entry->Key())) {
    return Optional<const DictEntry*>(found->get());
  } else {
    return Optional<const DictEntry*>::Null();
  }
}

LexiconPtr TextDict::GetLexicon() const { return lexicon; }

void TextDict::SerializeToFile(FILE* fp) const {
  for (const auto& entry : *lexicon) {
    fprintf(fp, "%s\n", entry->ToString().c_str());
  }
}
