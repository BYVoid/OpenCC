/*
 * Open Chinese Convert
 *
 * Copyright 2020-2026 Carbo Kuo and contributors
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

#include "Lexicon.hpp"

namespace opencc {

namespace {

DictEntry* ParseKeyValues(const char* buff, size_t lineNum,
                          char keyValueDelimiter) {
  size_t length;
  if (buff == nullptr || UTF8Util::IsLineEndingOrFileEnding(*buff)) {
    return nullptr;
  }
  const char* pbuff = UTF8Util::FindNextInline(buff, keyValueDelimiter);
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

} // namespace

void Lexicon::Sort() {
  std::sort(entries.begin(), entries.end(), DictEntry::UPtrLessThan);
}

bool Lexicon::IsSorted() {
  return std::is_sorted(entries.begin(), entries.end(),
                        DictEntry::UPtrLessThan);
}

bool Lexicon::IsUnique(std::string* dupkey) {
  for (size_t i = 1; i < entries.size(); ++i) {
    if (entries[i - 1]->Key() == entries[i]->Key()) {
      if (dupkey) {
        *dupkey = entries[i]->Key();
      }
      return false;
    }
  }
  return true;
}

LexiconPtr Lexicon::ParseLexiconFromFile(FILE* fp) {
  return ParseLexiconFromFile(fp, '\t');
}

LexiconPtr Lexicon::ParseLexiconFromFile(FILE* fp, char keyValueDelimiter) {
  const int ENTRY_BUFF_SIZE = 4096;
  char buff[ENTRY_BUFF_SIZE];
  LexiconPtr lexicon(new Lexicon);
  UTF8Util::SkipUtf8Bom(fp);

  size_t lineNum = 1;
  while (fgets(buff, ENTRY_BUFF_SIZE, fp)) {
    if (*buff == '#') {
      lineNum++;
      continue;
    }
    DictEntry* entry = ParseKeyValues(buff, lineNum, keyValueDelimiter);
    if (entry != nullptr) {
      lexicon->Add(entry);
    }
    lineNum++;
  }
  return lexicon;
}

LexiconPtr Lexicon::ParseLexiconFromBuffer(const char* data, size_t size) {
  LexiconPtr lexicon(new Lexicon);
  size_t offset = 0;
  if (size >= 3 && static_cast<unsigned char>(data[0]) == 0xef &&
      static_cast<unsigned char>(data[1]) == 0xbb &&
      static_cast<unsigned char>(data[2]) == 0xbf) {
    offset = 3;
  }

  size_t lineNum = 1;
  while (offset < size) {
    size_t lineEnd = offset;
    while (lineEnd < size && data[lineEnd] != '\n') {
      lineEnd++;
    }

    std::string line(data + offset, lineEnd - offset);
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    if (!line.empty() && line.front() != '#') {
      line.push_back('\n');
      DictEntry* entry = ParseKeyValues(line.c_str(), lineNum, '\t');
      if (entry != nullptr) {
        lexicon->Add(entry);
      }
    }

    offset = lineEnd < size ? lineEnd + 1 : lineEnd;
    lineNum++;
  }
  return lexicon;
}

} // namespace opencc
