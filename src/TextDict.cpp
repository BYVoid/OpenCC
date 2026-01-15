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
#include <map>

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

TextDict::TextDict(const LexiconPtr& _lexicon)
    : maxLength(GetKeyMaxLength(_lexicon)), lexicon(_lexicon) {
  assert(lexicon->IsSorted());
  assert(lexicon->IsUnique());
}

TextDict::~TextDict() {}

TextDictPtr TextDict::NewFromSortedFile(FILE* fp) {
  const LexiconPtr& lexicon = Lexicon::ParseLexiconFromFile(fp);
  return TextDictPtr(new TextDict(lexicon));
}

TextDictPtr TextDict::NewFromFile(FILE* fp) {
  const LexiconPtr& lexicon = Lexicon::ParseLexiconFromFile(fp);
  if (lexicon->HasAnnotations()) {
    lexicon->SortWithAnnotations();
  } else {
    lexicon->Sort();
  }
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
  if (!lexicon->HasAnnotations()) {
    // No annotations, use simple serialization
    for (const auto& entry : *lexicon) {
      fprintf(fp, "%s\n", entry->ToString().c_str());
    }
    return;
  }

  // Serialize with annotations
  const auto& headerBlocks = lexicon->GetHeaderBlocks();
  const auto& footerBlocks = lexicon->GetFooterBlocks();
  const auto& annotatedEntries = lexicon->GetAnnotatedEntries();
  const auto& floatingBlocks = lexicon->GetFloatingBlocks();

  // Write header blocks
  for (size_t i = 0; i < headerBlocks.size(); ++i) {
    for (const auto& line : headerBlocks[i].lines) {
      fprintf(fp, "%s\n", line.c_str());
    }
    // Add empty line after each header block
    if (i < headerBlocks.size() - 1) {
      fprintf(fp, "\n");
    }
  }

  // Add empty line after header if there were header blocks
  if (!headerBlocks.empty() && !annotatedEntries.empty()) {
    fprintf(fp, "\n");
  }

  // Group floating blocks by anchor index
  std::map<size_t, std::vector<const CommentBlock*>> floatingByAnchor;
  for (const auto& pair : floatingBlocks) {
    floatingByAnchor[pair.first].push_back(&pair.second);
  }

  // Write entries with their attached comments and floating blocks
  for (size_t i = 0; i < annotatedEntries.size(); ++i) {
    // Write floating blocks anchored before this entry
    auto floatIt = floatingByAnchor.find(i);
    if (floatIt != floatingByAnchor.end()) {
      for (const auto* block : floatIt->second) {
        // Ensure empty line before floating block
        fprintf(fp, "\n");
        for (const auto& line : block->lines) {
          fprintf(fp, "%s\n", line.c_str());
        }
        // Ensure empty line after floating block
        fprintf(fp, "\n");
      }
    }

    // Write attached comment if present
    if (annotatedEntries[i].attachedComment) {
      for (const auto& line : annotatedEntries[i].attachedComment->lines) {
        fprintf(fp, "%s\n", line.c_str());
      }
      // No empty line after attached comment (it must be directly before entry)
    }

    // Write the entry
    fprintf(fp, "%s\n", annotatedEntries[i].entry->ToString().c_str());
  }

  // Write floating blocks anchored after all entries
  auto floatIt = floatingByAnchor.find(annotatedEntries.size());
  if (floatIt != floatingByAnchor.end()) {
    for (const auto* block : floatIt->second) {
      fprintf(fp, "\n");
      for (const auto& line : block->lines) {
        fprintf(fp, "%s\n", line.c_str());
      }
    }
  }

  // Write footer blocks
  if (!footerBlocks.empty()) {
    // Add empty line before footer if there were entries
    if (!annotatedEntries.empty()) {
      fprintf(fp, "\n");
    }
    for (size_t i = 0; i < footerBlocks.size(); ++i) {
      for (const auto& line : footerBlocks[i].lines) {
        fprintf(fp, "%s\n", line.c_str());
      }
      if (i < footerBlocks.size() - 1) {
        fprintf(fp, "\n");
      }
    }
  }
}
