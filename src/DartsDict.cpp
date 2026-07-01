/*
 * Open Chinese Convert
 *
 * Copyright 2010-2026 Carbo Kuo and contributors
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
#include <cstdint>
#include <cstring>
#include <memory>

#include "BinaryDict.hpp"
#include "DartsDict.hpp"
#include "Dict.hpp"
#include "Lexicon.hpp"
#include "darts.h"

using namespace opencc;

static const char* OCDHEADER = "OPENCCDARTS1";

// Minimal reader for legacy OPENCCDARTS1 files built on 64-bit platforms,
// where id_type was size_t (8 bytes).  The bit layout of each unit is the
// same as the 32-bit case; old files simply zero-extend every unit to 64 bits.
namespace {

struct LegacyUnit64 {
  uint64_t unit;
  bool has_leaf() const { return ((unit >> 8) & 1) == 1; }
  int value() const {
    return static_cast<int>(unit & ((1U << 31) - 1));
  }
  uint64_t label() const { return unit & ((1U << 31) | 0xFF); }
  uint64_t offset() const {
    return (unit >> 10) << ((unit & (1U << 9)) >> 6);
  }
};

int Legacy64ExactMatch(const LegacyUnit64* arr, const char* key, size_t len) {
  size_t pos = 0;
  for (size_t i = 0; i < len; ++i) {
    uint64_t c = static_cast<unsigned char>(key[i]);
    pos ^= static_cast<size_t>(arr[pos].offset() ^ c);
    if (arr[pos].label() != c) return -1;
  }
  if (!arr[pos].has_leaf()) return -1;
  return arr[pos ^ static_cast<size_t>(arr[pos].offset())].value();
}

// Returns number of matches found; fills results[] with values (shortest to
// longest).  Mirror of Darts::DoubleArray::commonPrefixSearch value-only form.
size_t Legacy64PrefixSearch(const LegacyUnit64* arr, const char* key,
                             size_t maxLen, int* results, size_t maxResults) {
  size_t count = 0;
  size_t pos = 0;
  for (size_t i = 0; i < maxLen; ++i) {
    uint64_t c = static_cast<unsigned char>(key[i]);
    pos ^= static_cast<size_t>(arr[pos].offset() ^ c);
    if (arr[pos].label() != c) break;
    if (arr[pos].has_leaf()) {
      if (count < maxResults) {
        results[count] = arr[pos ^ static_cast<size_t>(arr[pos].offset())].value();
      }
      ++count;
    }
  }
  return count;
}

// Mirror of Darts::DoubleArray::validate(); checks root sanity, offset bounds,
// and leaf value bounds to catch malformed legacy files before any traversal.
bool ValidateLegacy64(const LegacyUnit64* arr, size_t numUnits, int maxValueLimit) {
  if (numUnits == 0) return false;
  if (arr[0].label() != 0 || arr[0].has_leaf() || arr[0].offset() == 0) return false;
  if (((static_cast<size_t>(arr[0].offset())) | 0xFFu) >= numUnits) return false;
  for (size_t i = 1; i < numUnits; ++i) {
    uint64_t lbl = arr[i].label();
    if (lbl <= 0xFF) {
      if (((i ^ static_cast<size_t>(arr[i].offset())) | 0xFFu) >= numUnits) return false;
    } else if (maxValueLimit >= 0) {
      if (arr[i].value() >= maxValueLimit) return false;
    }
  }
  return true;
}

}  // namespace

class DartsDict::DartsInternal {
public:
  BinaryDictPtr binary;
  void* buffer;
  Darts::DoubleArray* doubleArray;     // 32-bit files (new, or old 32-bit platform)
  const LegacyUnit64* legacyArray64;  // old 64-bit files; points into buffer

  DartsInternal()
      : binary(nullptr), buffer(nullptr), doubleArray(nullptr),
        legacyArray64(nullptr) {}

  ~DartsInternal() {
    if (buffer != nullptr) {
      free(buffer);
    }
    delete doubleArray;
    // legacyArray64 aliases buffer — no separate free
  }
};

DartsDict::DartsDict() { internal = new DartsInternal; }

DartsDict::~DartsDict() { delete internal; }

size_t DartsDict::KeyMaxLength() const { return maxLength; }

Optional<const DictEntry*> DartsDict::Match(const char* word,
                                            size_t len) const {
  if (len > maxLength) {
    return Optional<const DictEntry*>::Null();
  }
  if (internal->legacyArray64 != nullptr) {
    int val = Legacy64ExactMatch(internal->legacyArray64, word, len);
    if (val != -1) {
      return Optional<const DictEntry*>(lexicon->At(static_cast<size_t>(val)));
    }
    return Optional<const DictEntry*>::Null();
  }
  Darts::DoubleArray& dict = *internal->doubleArray;
  Darts::DoubleArray::result_pair_type result;
  dict.exactMatchSearch(word, result, len);
  if (result.value != -1) {
    return Optional<const DictEntry*>(
        lexicon->At(static_cast<size_t>(result.value)));
  }
  return Optional<const DictEntry*>::Null();
}

Optional<const DictEntry*> DartsDict::MatchPrefix(const char* word,
                                                  size_t len) const {
  const size_t DEFAULT_NUM_ENTRIES = 64;
  size_t searchLen = (std::min)(maxLength, len);

  if (internal->legacyArray64 != nullptr) {
    int results[DEFAULT_NUM_ENTRIES];
    size_t numMatched = Legacy64PrefixSearch(
        internal->legacyArray64, word, searchLen, results, DEFAULT_NUM_ENTRIES);
    if (numMatched == 0) {
      return Optional<const DictEntry*>::Null();
    }
    int maxVal;
    if (numMatched < DEFAULT_NUM_ENTRIES) {
      maxVal = results[numMatched - 1];
    } else {
      std::vector<int> rematchedResults(numMatched);
      numMatched = Legacy64PrefixSearch(internal->legacyArray64, word,
                                        searchLen, rematchedResults.data(),
                                        rematchedResults.size());
      maxVal = rematchedResults[numMatched - 1];
    }
    if (maxVal >= 0) {
      return Optional<const DictEntry*>(
          lexicon->At(static_cast<size_t>(maxVal)));
    }
    return Optional<const DictEntry*>::Null();
  }

  Darts::DoubleArray& dict = *internal->doubleArray;
  Darts::DoubleArray::value_type results[DEFAULT_NUM_ENTRIES];
  Darts::DoubleArray::value_type maxMatchedResult;
  size_t numMatched =
      dict.commonPrefixSearch(word, results, DEFAULT_NUM_ENTRIES, searchLen);
  if (numMatched == 0) {
    return Optional<const DictEntry*>::Null();
  } else if (numMatched < DEFAULT_NUM_ENTRIES) {
    maxMatchedResult = results[numMatched - 1];
  } else {
    Darts::DoubleArray::value_type* rematchedResults =
        new Darts::DoubleArray::value_type[numMatched];
    numMatched = dict.commonPrefixSearch(word, rematchedResults, numMatched,
                                         searchLen);
    maxMatchedResult = rematchedResults[numMatched - 1];
    delete[] rematchedResults;
  }
  if (maxMatchedResult >= 0) {
    return Optional<const DictEntry*>(
        lexicon->At(static_cast<size_t>(maxMatchedResult)));
  }
  return Optional<const DictEntry*>::Null();
}

LexiconPtr DartsDict::GetLexicon() const { return lexicon; }

PrefixMatchView DartsDict::MatchPrefixValue(const char* word,
                                            size_t len) const {
  Optional<const DictEntry*> matched = MatchPrefix(word, len);
  if (matched.IsNull()) {
    return PrefixMatchView{};
  }
  const DictEntry* entry = matched.Get();
  const size_t keyLen = entry->KeyLength();
  return PrefixMatchView{true, keyLen, std::string_view(word, keyLen),
                         entry->GetDefaultView()};
}

DartsDictPtr DartsDict::NewFromFile(FILE* fp) {
  DartsDictPtr dict(new DartsDict());
  auto internal = dict->internal;

  size_t headerLen = strlen(OCDHEADER);
  std::vector<char> headerBuffer(headerLen);
  size_t bytesRead = fread(headerBuffer.data(), sizeof(char), headerLen, fp);
  if (bytesRead != headerLen || memcmp(headerBuffer.data(), OCDHEADER, headerLen) != 0) {
    throw InvalidFormat("Invalid OpenCC dictionary header");
  }

  // Measure remaining bytes for bounds checking
  long currentOffset = ftell(fp);
  fseek(fp, 0L, SEEK_END);
  long fileEnd = ftell(fp);
  fseek(fp, currentOffset, SEEK_SET);
  size_t remainingSize =
      (fileEnd > currentOffset)
          ? static_cast<size_t>(fileEnd - currentOffset)
          : 0;

  // Detect 32-bit vs 64-bit unit size by reading 8 bytes and checking
  // whether bytes [4..7] are all zero.
  //   - Old 64-bit build: dartsSize field is uint64_t (8 bytes); high 32 bits
  //     are zero for any realistic file size → bytes [4..7] == 0.
  //   - 32-bit build (new or old): dartsSize field is uint32_t (4 bytes);
  //     bytes [4..7] are the first 4 bytes of the darts array (non-zero for
  //     any valid array whose root unit has a non-zero offset).
  uint8_t probe[8];
  if (fread(probe, 1, 8, fp) != 8) {
    throw InvalidFormat("Invalid OpenCC dictionary header (dartsSize)");
  }
  bool is64bit =
      (probe[4] == 0 && probe[5] == 0 && probe[6] == 0 && probe[7] == 0);

  if (is64bit) {
    uint64_t dartsSize64;
    memcpy(&dartsSize64, probe, 8);
    size_t dartsSize = static_cast<size_t>(dartsSize64);
    if (dartsSize > remainingSize) {
      throw InvalidFormat(
          "Invalid OpenCC dictionary (dartsSize exceeds file size)");
    }
    if (dartsSize % sizeof(LegacyUnit64) != 0) {
      throw InvalidFormat("Invalid legacy OCD dictionary unit alignment");
    }
    std::unique_ptr<void, decltype(&free)> buffer(malloc(dartsSize), free);
    if (!buffer) {
      throw std::bad_alloc();
    }
    bytesRead = fread(buffer.get(), 1, dartsSize, fp);
    if (bytesRead != dartsSize) {
      throw InvalidFormat("Invalid legacy OCD dictionary size mismatch");
    }

    BinaryDictPtr binary = BinaryDict::NewFromFile(fp);

    internal->buffer = buffer.release();
    internal->binary = std::move(binary);
    internal->legacyArray64 = static_cast<const LegacyUnit64*>(internal->buffer);

    dict->lexicon = internal->binary->GetLexicon();
    dict->maxLength = internal->binary->KeyMaxLength();
    size_t numUnits = dartsSize / sizeof(LegacyUnit64);
    if (!ValidateLegacy64(internal->legacyArray64, numUnits,
                          static_cast<int>(dict->lexicon->Length()))) {
      throw InvalidFormat("Invalid legacy OCD dictionary darts data");
    }
    return dict;
  }

  // 32-bit: dartsSize is the first 4 bytes of probe; remaining 4 bytes belong
  // to the array, so seek back.
  fseek(fp, -4, SEEK_CUR);
  uint32_t dartsSize32;
  memcpy(&dartsSize32, probe, 4);
  size_t dartsSize = dartsSize32;
  if (dartsSize > remainingSize) {
    throw InvalidFormat(
        "Invalid OpenCC dictionary (dartsSize exceeds file size)");
  }
  std::unique_ptr<Darts::DoubleArray> doubleArray(new Darts::DoubleArray());
  if (dartsSize % doubleArray->unit_size() != 0) {
    throw InvalidFormat("Invalid OpenCC dictionary size of darts alignment");
  }
  std::unique_ptr<void, decltype(&free)> buffer(malloc(dartsSize), free);
  if (!buffer) {
    throw std::bad_alloc();
  }
  bytesRead = fread(buffer.get(), 1, dartsSize, fp);
  if (bytesRead != dartsSize) {
    throw InvalidFormat("Invalid OpenCC dictionary size of darts mismatch");
  }
  doubleArray->set_array(buffer.get(), dartsSize / doubleArray->unit_size());

  BinaryDictPtr binary = BinaryDict::NewFromFile(fp);

  internal->buffer = buffer.release();
  internal->doubleArray = doubleArray.release();
  internal->binary = std::move(binary);

  dict->lexicon = internal->binary->GetLexicon();
  dict->maxLength = internal->binary->KeyMaxLength();
  if (!internal->doubleArray->validate(static_cast<Darts::DoubleArray::value_type>(
          dict->lexicon->Length()))) {
    throw InvalidFormat("Invalid OpenCC dictionary darts data");
  }
  return dict;
}

DartsDictPtr DartsDict::NewFromBuffer(const char* data, size_t size) {
  DartsDictPtr dict(new DartsDict());
  auto internal = dict->internal;

  size_t headerLen = strlen(OCDHEADER);
  if (size < headerLen || memcmp(data, OCDHEADER, headerLen) != 0) {
    throw InvalidFormat("Invalid OpenCC dictionary header");
  }
  size_t offset = headerLen;

  if (size - offset < 8) {
    throw InvalidFormat("Invalid OpenCC dictionary header (dartsSize)");
  }
  uint8_t probe[8];
  memcpy(probe, data + offset, 8);
  offset += 8;
  bool is64bit =
      (probe[4] == 0 && probe[5] == 0 && probe[6] == 0 && probe[7] == 0);

  if (is64bit) {
    uint64_t dartsSize64;
    memcpy(&dartsSize64, probe, 8);
    size_t dartsSize = static_cast<size_t>(dartsSize64);
    if (dartsSize > size - offset) {
      throw InvalidFormat(
          "Invalid OpenCC dictionary (dartsSize exceeds file size)");
    }
    if (dartsSize % sizeof(LegacyUnit64) != 0) {
      throw InvalidFormat("Invalid legacy OCD dictionary unit alignment");
    }
    std::unique_ptr<void, decltype(&free)> buffer(malloc(dartsSize), free);
    if (!buffer) {
      throw std::bad_alloc();
    }
    memcpy(buffer.get(), data + offset, dartsSize);
    offset += dartsSize;

    BinaryDictPtr binary = BinaryDict::NewFromBuffer(data + offset, size - offset);

    internal->buffer = buffer.release();
    internal->binary = std::move(binary);
    internal->legacyArray64 = static_cast<const LegacyUnit64*>(internal->buffer);

    dict->lexicon = internal->binary->GetLexicon();
    dict->maxLength = internal->binary->KeyMaxLength();
    size_t numUnits = dartsSize / sizeof(LegacyUnit64);
    if (!ValidateLegacy64(internal->legacyArray64, numUnits,
                          static_cast<int>(dict->lexicon->Length()))) {
      throw InvalidFormat("Invalid legacy OCD dictionary darts data");
    }
    return dict;
  }

  // 32-bit:
  uint32_t dartsSize32;
  memcpy(&dartsSize32, probe, 4);
  size_t dartsSize = dartsSize32;
  offset -= 4;
  if (dartsSize > size - offset) {
    throw InvalidFormat(
        "Invalid OpenCC dictionary (dartsSize exceeds file size)");
  }
  std::unique_ptr<Darts::DoubleArray> doubleArray(new Darts::DoubleArray());
  if (dartsSize % doubleArray->unit_size() != 0) {
    throw InvalidFormat("Invalid OpenCC dictionary size of darts alignment");
  }
  std::unique_ptr<void, decltype(&free)> buffer(malloc(dartsSize), free);
  if (!buffer) {
    throw std::bad_alloc();
  }
  memcpy(buffer.get(), data + offset, dartsSize);
  offset += dartsSize;
  doubleArray->set_array(buffer.get(), dartsSize / doubleArray->unit_size());

  BinaryDictPtr binary = BinaryDict::NewFromBuffer(data + offset, size - offset);

  internal->buffer = buffer.release();
  internal->doubleArray = doubleArray.release();
  internal->binary = std::move(binary);

  dict->lexicon = internal->binary->GetLexicon();
  dict->maxLength = internal->binary->KeyMaxLength();
  if (!internal->doubleArray->validate(static_cast<Darts::DoubleArray::value_type>(
          dict->lexicon->Length()))) {
    throw InvalidFormat("Invalid OpenCC dictionary darts data");
  }
  return dict;
}

DartsDictPtr DartsDict::NewFromDict(const Dict& thatDict) {
  DartsDictPtr dict(new DartsDict());
  auto internal = dict->internal;

  std::unique_ptr<Darts::DoubleArray> doubleArray(new Darts::DoubleArray());
  std::vector<std::string> keys;
  std::vector<const char*> keys_cstr;
  size_t maxLength = 0;
  const LexiconPtr& lexicon = thatDict.GetLexicon();
  size_t lexiconCount = lexicon->Length();
  keys.resize(lexiconCount);
  keys_cstr.resize(lexiconCount);
  for (size_t i = 0; i < lexiconCount; i++) {
    const DictEntry* entry = lexicon->At(i);
    keys[i] = entry->Key();
    keys_cstr[i] = keys[i].c_str();
    maxLength = (std::max)(entry->KeyLength(), maxLength);
  }
  doubleArray->build(lexicon->Length(), &keys_cstr[0]);
  dict->lexicon = lexicon;
  dict->maxLength = maxLength;
  internal->doubleArray = doubleArray.release();
  return dict;
}

void DartsDict::SerializeToFile(FILE* fp) const {
  if (internal->doubleArray == nullptr) {
    throw InvalidFormat(
        "Cannot serialize a legacy 64-bit OCD dictionary; "
        "rebuild via NewFromDict first");
  }
  Darts::DoubleArray& dict = *internal->doubleArray;

  fwrite(OCDHEADER, sizeof(char), strlen(OCDHEADER), fp);

  // Write dartsSize as uint32_t so the file is platform-independent.
  // id_type is fixed at uint32_t, so total_size() always fits in 32 bits.
  uint32_t dartsSize = static_cast<uint32_t>(dict.total_size());
  fwrite(&dartsSize, sizeof(uint32_t), 1, fp);
  fwrite(dict.array(), sizeof(char), dartsSize, fp);

  internal->binary.reset(new BinaryDict(lexicon));
  internal->binary->SerializeToFile(fp);
}
