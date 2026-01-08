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

#include <algorithm>
#include <map>

#include "Lexicon.hpp"

namespace opencc {

namespace {

enum class LineType { Empty, Comment, Entry };

struct ParsedLine {
  LineType type;
  std::string content;     // Raw line content
  DictEntry* entry;        // Parsed entry (nullptr for non-entry lines)

  ParsedLine() : type(LineType::Empty), entry(nullptr) {}
};

// Determine line type when preserving comments
LineType DetermineLineType(const char* buff) {
  if (buff == nullptr || UTF8Util::IsLineEndingOrFileEnding(*buff)) {
    return LineType::Empty;
  }
  // Comment lines start with #
  if (*buff == '#') {
    return LineType::Comment;
  }
  // Check if it's an entry line (must have a tab)
  const char* pbuff = UTF8Util::FindNextInline(buff, '\t');
  if (!UTF8Util::IsLineEndingOrFileEnding(*pbuff)) {
    return LineType::Entry;
  }
  // Line with content but no tab - could be empty or malformed
  // Check if it's all whitespace
  const char* p = buff;
  while (!UTF8Util::IsLineEndingOrFileEnding(*p)) {
    if (*p != ' ' && *p != '\t') {
      // Non-whitespace character without tab = malformed
      return LineType::Entry; // Will fail in ParseKeyValues
    }
    p++;
  }
  return LineType::Empty;
}

DictEntry* ParseKeyValues(const char* buff, size_t lineNum) {
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

std::string TrimLineEnding(const char* buff) {
  std::string line(buff);
  // Remove trailing \r\n or \n
  while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
    line.pop_back();
  }
  return line;
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

LexiconPtr Lexicon::ParseLexiconFromFile(FILE* fp, bool preserveComments) {
  const int ENTRY_BUFF_SIZE = 4096;
  char buff[ENTRY_BUFF_SIZE];
  LexiconPtr lexicon(new Lexicon);
  UTF8Util::SkipUtf8Bom(fp);

  // If not preserving comments, use simple parsing (original behavior)
  if (!preserveComments) {
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

  // Preserve comments: use detailed parsing
  std::vector<ParsedLine> allLines;
  size_t lineNum = 1;

  // Phase 1: Parse all lines and determine their types
  while (fgets(buff, ENTRY_BUFF_SIZE, fp)) {
    ParsedLine line;
    line.type = DetermineLineType(buff);
    line.content = TrimLineEnding(buff);

    if (line.type == LineType::Entry) {
      line.entry = ParseKeyValues(buff, lineNum);
      if (line.entry != nullptr) {
        lexicon->Add(line.entry);
      }
    }

    allLines.push_back(std::move(line));
    lineNum++;
  }

  // Phase 2: Build comment blocks and classify them
  std::vector<CommentBlock> headerBlocks;
  std::vector<CommentBlock> footerBlocks;
  std::vector<AnnotatedEntry> annotatedEntries;
  std::vector<std::pair<size_t, CommentBlock>> floatingBlocks; // (anchor_idx, block)

  // Find first and last entry line indices
  int firstEntryIdx = -1;
  int lastEntryIdx = -1;
  for (size_t i = 0; i < allLines.size(); ++i) {
    if (allLines[i].type == LineType::Entry && allLines[i].entry != nullptr) {
      if (firstEntryIdx == -1) {
        firstEntryIdx = static_cast<int>(i);
      }
      lastEntryIdx = static_cast<int>(i);
    }
  }

  if (firstEntryIdx == -1) {
    // No entries, all comments are header or footer
    // For simplicity, treat them as header
    std::vector<std::string> commentLines;
    for (const auto& line : allLines) {
      if (line.type == LineType::Comment) {
        commentLines.push_back(line.content);
      } else if (line.type == LineType::Empty && !commentLines.empty()) {
        headerBlocks.emplace_back(std::move(commentLines));
        commentLines.clear();
      }
    }
    if (!commentLines.empty()) {
      headerBlocks.emplace_back(std::move(commentLines));
    }
    lexicon->SetHeaderBlocks(std::move(headerBlocks));
    return lexicon;
  }

  // Find the last empty line before first entry
  int headerEndIdx = -1;
  for (int i = firstEntryIdx - 1; i >= 0; --i) {
    if (allLines[i].type == LineType::Empty) {
      headerEndIdx = i;
      break;
    }
  }

  // Build header blocks (before headerEndIdx)
  std::vector<std::string> currentBlock;
  for (int i = 0; i <= headerEndIdx; ++i) {
    if (allLines[i].type == LineType::Comment) {
      currentBlock.push_back(allLines[i].content);
    } else if (allLines[i].type == LineType::Empty) {
      if (!currentBlock.empty()) {
        headerBlocks.emplace_back(std::move(currentBlock));
        currentBlock.clear();
      }
    }
  }
  if (!currentBlock.empty()) {
    headerBlocks.emplace_back(std::move(currentBlock));
    currentBlock.clear();
  }

  // Build footer blocks (after lastEntryIdx)
  for (size_t i = lastEntryIdx + 1; i < allLines.size(); ++i) {
    if (allLines[i].type == LineType::Comment) {
      currentBlock.push_back(allLines[i].content);
    } else if (allLines[i].type == LineType::Empty) {
      if (!currentBlock.empty()) {
        footerBlocks.emplace_back(std::move(currentBlock));
        currentBlock.clear();
      }
    }
  }
  if (!currentBlock.empty()) {
    footerBlocks.emplace_back(std::move(currentBlock));
  }

  // Build annotated entries (between first and last entry)
  // Scan from headerEndIdx+1 to lastEntryIdx
  size_t entryIndex = 0;
  for (int i = headerEndIdx + 1; i <= lastEntryIdx; ++i) {
    if (allLines[i].type == LineType::Comment) {
      currentBlock.push_back(allLines[i].content);
    } else if (allLines[i].type == LineType::Entry && allLines[i].entry != nullptr) {
      // Check if current comment block should attach to this entry
      CommentBlock* attachedComment = nullptr;
      if (!currentBlock.empty()) {
        // Check if there's an empty line between comment and entry
        bool hasEmptyLineBetween = false;
        for (int j = i - 1; j >= 0 && allLines[j].type != LineType::Entry; --j) {
          if (allLines[j].type == LineType::Empty) {
            hasEmptyLineBetween = true;
            break;
          }
          if (allLines[j].type == LineType::Comment) {
            break; // reached the comment block
          }
        }

        if (!hasEmptyLineBetween) {
          // Attached comment
          attachedComment = new CommentBlock(std::move(currentBlock));
        } else {
          // Floating comment
          floatingBlocks.emplace_back(entryIndex, CommentBlock(currentBlock));
        }
        currentBlock.clear();
      }

      // Create annotated entry
      DictEntry* entryCopy = DictEntryFactory::New(allLines[i].entry);
      annotatedEntries.emplace_back(entryCopy, attachedComment);
      entryIndex++;
    } else if (allLines[i].type == LineType::Empty) {
      if (!currentBlock.empty()) {
        // Comment block followed by empty line - it's floating
        // Find next entry to determine anchor
        size_t anchorIdx = entryIndex;
        for (int j = i + 1; j <= lastEntryIdx; ++j) {
          if (allLines[j].type == LineType::Entry && allLines[j].entry != nullptr) {
            break; // anchorIdx is already correct
          }
        }
        floatingBlocks.emplace_back(anchorIdx, CommentBlock(currentBlock));
        currentBlock.clear();
      }
    }
  }

  // Handle any remaining comment block as floating
  if (!currentBlock.empty()) {
    floatingBlocks.emplace_back(entryIndex, CommentBlock(currentBlock));
  }

  // Store results
  lexicon->SetHeaderBlocks(std::move(headerBlocks));
  lexicon->SetFooterBlocks(std::move(footerBlocks));
  lexicon->SetAnnotatedEntries(std::move(annotatedEntries));
  lexicon->SetFloatingBlocks(std::move(floatingBlocks));

  return lexicon;
}

void Lexicon::SortWithAnnotations() {
  if (!HasAnnotations() || annotatedEntries.empty()) {
    // No annotations, just sort entries normally
    Sort();
    return;
  }

  // Create a mapping from old entry pointers to their annotated counterparts
  std::map<std::string, size_t> keyToAnnotatedIndex;
  for (size_t i = 0; i < annotatedEntries.size(); ++i) {
    keyToAnnotatedIndex[annotatedEntries[i].Key()] = i;
  }

  // Sort the regular entries
  Sort();

  // Rebuild annotatedEntries in the new order
  std::vector<AnnotatedEntry> sortedAnnotated;
  sortedAnnotated.reserve(annotatedEntries.size());

  for (const auto& entry : entries) {
    auto it = keyToAnnotatedIndex.find(entry->Key());
    if (it != keyToAnnotatedIndex.end()) {
      size_t oldIndex = it->second;
      // Move the annotated entry (with its comment) to the new sorted order
      DictEntry* entryCopy = DictEntryFactory::New(entry.get());
      CommentBlock* commentCopy = nullptr;
      if (annotatedEntries[oldIndex].attachedComment) {
        commentCopy = new CommentBlock(annotatedEntries[oldIndex].attachedComment->lines);
      }
      sortedAnnotated.emplace_back(entryCopy, commentCopy);
    } else {
      // Entry without annotation
      DictEntry* entryCopy = DictEntryFactory::New(entry.get());
      sortedAnnotated.emplace_back(entryCopy, nullptr);
    }
  }

  annotatedEntries = std::move(sortedAnnotated);

  // Floating blocks' anchor indices remain valid as they refer to the sorted position
  // No need to update floatingBlocks
}

} // namespace opencc
