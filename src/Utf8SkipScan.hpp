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

// The NEON movemask substitute (vshrn + ctz) assumes little-endian lane
// ordering, so big-endian ARM (rare, but existing in some 32-bit embedded
// toolchains) must fall back to the endian-checked SWAR path below. SSE2 and
// WASM SIMD128 targets are little-endian by definition.
#if defined(__SSE2__) || defined(_M_X64) ||                                    \
    (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#define OPENCC_UTF8_SKIP_SCAN_SSE2 1
#include <emmintrin.h>
#elif (defined(__aarch64__) || defined(_M_ARM64) || defined(__ARM_NEON)) &&    \
    ((defined(__BYTE_ORDER__) &&                                               \
      __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) ||                            \
     defined(_M_ARM64))
#define OPENCC_UTF8_SKIP_SCAN_NEON 1
#include <arm_neon.h>
#elif defined(__wasm_simd128__)
#define OPENCC_UTF8_SKIP_SCAN_WASM 1
#include <wasm_simd128.h>
#endif

#if defined(_MSC_VER)
#include <intrin.h>
#endif

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
 * character, and to vectorize the common all-ASCII case.
 */
struct Utf8SkipTable {
  /**
   * candidate[b] is true when byte value b may begin a dictionary key (or is
   * the lead byte of an ideographic description operator), so the conversion
   * loop must run a full prefix lookup at that position.
   */
  bool candidate[256] = {};
  /**
   * True when any ASCII byte value is a candidate; disables the vectorized
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

inline size_t CountTrailingZeros(uint64_t mask) {
#if defined(__GNUC__) || defined(__clang__)
  return static_cast<size_t>(__builtin_ctzll(mask));
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
  unsigned long index = 0;
  _BitScanForward64(&index, mask);
  return static_cast<size_t>(index);
#else
  size_t count = 0;
  while ((mask & 1) == 0) {
    mask >>= 1;
    count++;
  }
  return count;
#endif
}

/**
 * Returns the number of leading bytes of [str, str + len) with the high bit
 * clear, i.e. the length of the leading ASCII run.
 */
inline size_t AsciiRunLength(const char* str, size_t len) {
  size_t pos = 0;
#if defined(OPENCC_UTF8_SKIP_SCAN_SSE2)
  for (; pos + 16 <= len; pos += 16) {
    const __m128i chunk =
        _mm_loadu_si128(reinterpret_cast<const __m128i*>(str + pos));
    const uint32_t mask = static_cast<uint32_t>(_mm_movemask_epi8(chunk));
    if (mask != 0) {
      return pos + CountTrailingZeros(mask);
    }
  }
#elif defined(OPENCC_UTF8_SKIP_SCAN_NEON)
  for (; pos + 16 <= len; pos += 16) {
    const uint8x16_t chunk =
        vld1q_u8(reinterpret_cast<const uint8_t*>(str + pos));
    // 0xFF per non-ASCII byte, then narrow to 4 bits per byte to obtain a
    // 64-bit mask (the NEON substitute for _mm_movemask_epi8).
    const uint8x16_t high =
        vcltq_s8(vreinterpretq_s8_u8(chunk), vdupq_n_s8(0));
    const uint8x8_t narrowed = vshrn_n_u16(vreinterpretq_u16_u8(high), 4);
    const uint64_t mask = vget_lane_u64(vreinterpret_u64_u8(narrowed), 0);
    if (mask != 0) {
      return pos + (CountTrailingZeros(mask) >> 2);
    }
  }
#elif defined(OPENCC_UTF8_SKIP_SCAN_WASM)
  for (; pos + 16 <= len; pos += 16) {
    const v128_t chunk = wasm_v128_load(str + pos);
    const uint32_t mask = static_cast<uint32_t>(wasm_i8x16_bitmask(chunk));
    if (mask != 0) {
      return pos + CountTrailingZeros(mask);
    }
  }
#else
  // SWAR fallback: examine 8 bytes per iteration.
  for (; pos + 8 <= len; pos += 8) {
    uint64_t word;
    std::memcpy(&word, str + pos, sizeof(word));
    const uint64_t mask = word & UINT64_C(0x8080808080808080);
    if (mask != 0) {
#if (defined(__BYTE_ORDER__) &&                                                \
     __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) ||                             \
    defined(_MSC_VER)
      return pos + CountTrailingZeros(mask) / 8;
#else
      break;
#endif
    }
  }
#endif
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
