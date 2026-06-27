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

#include "Converter.hpp"
#include "UTF8Util.hpp"

using namespace opencc;

std::string ConverterStream::ConvertChunk(std::string_view input) {
  if (!input.empty()) {
    pending.append(input);
  }
  if (pending.empty()) {
    return std::string();
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

  if (keepStart == bufferBegin) {
    return std::string();
  }

  const std::string output = converter->Convert(std::string_view(
      bufferBegin, static_cast<size_t>(keepStart - bufferBegin)));
  pending.erase(0, static_cast<size_t>(keepStart - bufferBegin));
  return output;
}

std::string ConverterStream::Finish(std::string_view input) {
  if (!input.empty()) {
    pending.append(input);
  }
  return Finish();
}

std::string ConverterStream::Finish() {
  const std::string output =
      pending.empty() ? std::string()
                      : converter->Convert(std::string_view(pending));
  pending.clear();
  return output;
}

