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

#include <cstddef>
#include <string_view>

#include "UTF8Util.hpp"

namespace opencc {
namespace internal {

/**
 * Default keep-tail window for streaming converters, in Unicode code
 * points.  Must equal ConverterStream's default maxKeepChars
 * (Converter.hpp, an installed header that cannot reference this private
 * one); ConversionAmbiguitiesTest pins the two defaults together
 * behaviorally, so a drift fails tests.
 */
inline constexpr size_t kDefaultStreamKeepChars = 16;

/**
 * Returns the number of bytes at the front of @p pending that a streaming
 * converter may flush now, or 0 if everything must be kept.
 *
 * The boundary excludes a trailing incomplete UTF-8 sequence, then withholds
 * the last @p maxKeepChars complete code points (so a phrase straddling two
 * consecutive chunks is not split across separate conversions), extending the
 * kept tail further if it ends in an incomplete ideographic description
 * sequence prefix.
 *
 * Shared by ConverterStream and AmbiguityStream so that both always flush on
 * identical boundaries; a windowing fix applied here reaches every streaming
 * wrapper at once.  Private (non-installed) header.
 */
inline size_t FlushableByteCount(std::string_view pending,
                                 size_t maxKeepChars) {
  if (pending.empty()) {
    return 0;
  }

  const char* bufferBegin = pending.data();
  const char* bufferEnd = bufferBegin + pending.size();
  const char* completeEnd = bufferBegin;
  while (completeEnd < bufferEnd) {
    const size_t nextCharLen = UTF8Util::NextCharLength(completeEnd);
    if (completeEnd + nextCharLen > bufferEnd) {
      break;
    }
    completeEnd += nextCharLen;
  }

  const char* keepStart = completeEnd;
  size_t charsKept = 0;
  while (keepStart > bufferBegin && charsKept < maxKeepChars) {
    const size_t prevCharLen = UTF8Util::PrevCharLength(keepStart);
    keepStart -= prevCharLen;
    charsKept++;
  }

  const char* idsKeepStart = completeEnd;
  const char* idsCandidate = completeEnd;
  size_t idsCharsScanned = 0;
  const size_t kMaxIDSCodePoints = 64;
  while (idsCandidate > bufferBegin && idsCharsScanned < kMaxIDSCodePoints) {
    idsCandidate -= UTF8Util::PrevCharLength(idsCandidate);
    idsCharsScanned++;
    if (UTF8Util::IsIncompleteIdeographicDescriptionSequencePrefix(
            idsCandidate, completeEnd - idsCandidate)) {
      idsKeepStart = idsCandidate;
    }
  }
  if (idsKeepStart < keepStart) {
    keepStart = idsKeepStart;
  }

  return static_cast<size_t>(keepStart - bufferBegin);
}

} // namespace internal
} // namespace opencc
