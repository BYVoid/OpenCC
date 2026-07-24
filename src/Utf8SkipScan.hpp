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

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include "UTF8Util.hpp"

namespace opencc {
namespace internal {

/**
 * Decodes the code point of a 2- or 3-byte UTF-8 sequence from raw bytes,
 * without validating continuation bytes (same masking as
 * UTF8Util::CodePointNoException). Both the skip-table builder and the
 * scanner must use this one decoder so that a key's first character always
 * maps to the same bitmap bit as the text it can match.
 */
inline uint32_t DecodeCodePoint23(const char* str, size_t charLength) {
  const unsigned char lead = static_cast<unsigned char>(str[0]);
  if (charLength == 2) {
    return ((lead & 0x1FU) << 6) |
           (static_cast<unsigned char>(str[1]) & 0x3FU);
  }
  return ((lead & 0x0FU) << 12) |
         ((static_cast<unsigned char>(str[1]) & 0x3FU) << 6) |
         (static_cast<unsigned char>(str[2]) & 0x3FU);
}

/**
 * Per-byte lookup table describing which byte values may begin a dictionary
 * key. The conversion hot loop uses it to consume runs of characters that
 * cannot possibly match any key without paying for a trie lookup per
 * character, and to scan the common all-ASCII case a word at a time.
 */
struct Utf8SkipTable {
  /**
   * candidate[b] is true when byte value b may begin a dictionary key (or is
   * the lead byte of an ideographic description operator), so the conversion
   * loop must run a full prefix lookup at that position.
   */
  bool candidate[256] = {};
  /**
   * True when any ASCII byte value is a candidate; disables the bulk
   * ASCII-run scan.
   */
  bool asciiHasCandidates = false;
  /**
   * Optional character-level refinement: one bit per BMP code point
   * (U+0000..U+FFFF, 8 KiB). When non-empty, 2- and 3-byte UTF-8 characters
   * are filtered by their exact code point instead of their lead byte, so
   * e.g. a rare CJK character sharing its lead byte with common dictionary
   * keys can still be skipped. 4-byte characters keep lead-byte filtering.
   * Character-level mode is on exactly when this vector is non-empty; there
   * is deliberately no separate flag to keep in sync.
   */
  std::vector<uint64_t> bmpCandidates;

  bool CharLevel() const { return !bmpCandidates.empty(); }

  void EnableCharLevel() { bmpCandidates.assign(0x10000 / 64, 0); }

  /**
   * Permanently falls back to lead-byte filtering, e.g. when a key's first
   * character is truncated or invalid and cannot be represented as a code
   * point. Byte-level candidates accumulated so far remain valid.
   */
  void DisableCharLevel() { bmpCandidates.clear(); }

  void MarkCharCandidate(uint32_t codePoint) {
    if (CharLevel() && codePoint < 0x10000) {
      bmpCandidates[codePoint >> 6] |= uint64_t(1) << (codePoint & 63);
    }
  }

  bool IsCharCandidate(uint32_t codePoint) const {
    assert(CharLevel());
    return (bmpCandidates[codePoint >> 6] >> (codePoint & 63)) & 1;
  }

  void MarkAllCandidates() {
    for (size_t b = 0; b < 256; b++) {
      candidate[b] = true;
    }
    DisableCharLevel();
  }

  /**
   * Must be called after candidate[] is filled and before the table is used.
   * Ideographic description operators must always stop the scan: the
   * conversion loop groups IDS sequences without consulting the dictionary,
   * and skipping over an operator would lose that grouping. The operator
   * set is defined by UTF8Util::IdeographicDescriptionOperatorArity() and
   * bounded by UTF8Util::kFirst/kLastIdeographicDescriptionOperator.
   */
  void Finalize() {
    static_assert(UTF8Util::kFirstIdeographicDescriptionOperator >= 0x0800 &&
                      UTF8Util::kLastIdeographicDescriptionOperator <= 0xFFFF,
                  "IDS operator range must lie in the 3-byte UTF-8 block");
    for (uint32_t cp = UTF8Util::kFirstIdeographicDescriptionOperator;
         cp <= UTF8Util::kLastIdeographicDescriptionOperator; cp++) {
      if (UTF8Util::IdeographicDescriptionOperatorArity(cp) == 0) {
        continue;
      }
      // The operator range lies in the 3-byte UTF-8 block of the BMP.
      candidate[0xE0 | (cp >> 12)] = true;
      MarkCharCandidate(cp);
    }
    asciiHasCandidates = false;
    for (size_t b = 0; b < 0x80; b++) {
      if (candidate[b]) {
        asciiHasCandidates = true;
        break;
      }
    }
  }
};

/**
 * Returns the number of leading bytes of [str, str + len) with the high bit
 * clear, i.e. the length of the leading ASCII run.
 *
 * Word-at-a-time (SWAR) scan in plain C++: no intrinsics, no architecture
 * or endianness special cases. Hand-written SSE2/NEON/WASM-SIMD kernels
 * were measured against this and won by at most 3% end to end (see the
 * commit that removed them): the candidate table has already eliminated the
 * per-character trie lookups that dominated, so widening the scan from 8 to
 * 16 bytes per step no longer moves the total.
 */
inline size_t AsciiRunLength(const char* str, size_t len) {
  size_t pos = 0;
  for (; pos + sizeof(uint64_t) <= len; pos += sizeof(uint64_t)) {
    uint64_t word;
    std::memcpy(&word, str + pos, sizeof(word));
    if ((word & UINT64_C(0x8080808080808080)) != 0) {
      break;
    }
  }
  // Resolve the exact position within the word that stopped the loop (at
  // most 8 iterations), and handle the trailing partial word.
  for (; pos < len; pos++) {
    if (static_cast<unsigned char>(str[pos]) >= 0x80) {
      break;
    }
  }
  return pos;
}

/**
 * Returns the number of leading bytes of [str, str + len) that the conversion
 * loop may consume without any dictionary lookup: whole UTF-8 characters
 * whose lead byte is not a candidate key-start byte. Scanning stops at the
 * first candidate lead byte and at invalid or truncated sequences (which the
 * caller's per-character path must handle).
 */
inline size_t SkipNonCandidateBytes(const Utf8SkipTable& table, const char* str,
                                    size_t len) {
  size_t pos = 0;
  while (pos < len) {
    const unsigned char lead = static_cast<unsigned char>(str[pos]);
    if (lead < 0x80) {
      if (!table.asciiHasCandidates) {
        pos += AsciiRunLength(str + pos, len - pos);
        continue;
      }
      if (table.candidate[lead]) {
        break;
      }
      pos++;
      continue;
    }
    const size_t charLength = UTF8Util::NextCharLengthNoException(str + pos);
    if (charLength == 0 || charLength > len - pos) {
      break;
    }
    if (table.CharLevel() && (charLength == 2 || charLength == 3)) {
      // DecodeCodePoint23 does not validate continuation bytes; the table
      // builder decodes key bytes with the same function, so a position is
      // skipped only when its exact bytes cannot begin any key. Either way
      // the scan consumes the same charLength bytes the per-character path
      // would, so behavior stays equivalent even on malformed input.
      if (table.IsCharCandidate(DecodeCodePoint23(str + pos, charLength))) {
        break;
      }
    } else if (table.candidate[lead]) {
      break;
    }
    pos += charLength;
  }
  return pos;
}

} // namespace internal
} // namespace opencc
