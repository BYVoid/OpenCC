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

#include <cstring>

#include "ConversionChain.hpp"
#include "ConversionInspection.hpp"
#include "Converter.hpp"
#include "Segments.hpp"
#include "UTF8Util.hpp"

using namespace opencc;

std::string Converter::Convert(const std::string& text) const {
  const SegmentsPtr& segments = segmentation->Segment(text);
  std::string converted;
  converted.reserve(text.length() + text.length() / 5);
  for (const char* segment : *segments) {
    conversionChain->AppendConvertedSegment(segment, &converted);
  }
  return converted;
}

size_t Converter::Convert(const char* input, char* output) const {
  const std::string& converted = Convert(input);
  strcpy(output, converted.c_str());
  return converted.length();
}

ConversionInspectionResult Converter::Inspect(const std::string& text) const {
  ConversionInspectionResult result;
  result.input = text;

  const SegmentsPtr& initialSegments = segmentation->Segment(text);
  result.segments = initialSegments->ToVector();

  const std::vector<SegmentsPtr> trace =
      conversionChain->ConvertWithTrace(initialSegments);

  result.stages.reserve(trace.size());
  for (size_t i = 0; i < trace.size(); i++) {
    ConversionInspectionStage stage;
    stage.index = i + 1;
    stage.segments = trace[i]->ToVector();
    result.stages.push_back(std::move(stage));
  }

  if (!trace.empty()) {
    result.output = trace.back()->ToString();
  } else {
    result.output = initialSegments->ToString();
  }

  return result;
}

std::string ConverterStream::ConvertChunk(const char* input, size_t length) {
  if (length > 0) {
    pending.append(input, length);
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

  if (keepStart == bufferBegin) {
    return std::string();
  }

  const std::string output = converter->Convert(
      std::string(bufferBegin, static_cast<size_t>(keepStart - bufferBegin)));
  pending.erase(0, static_cast<size_t>(keepStart - bufferBegin));
  return output;
}

std::string ConverterStream::Finish() {
  const std::string output = pending.empty() ? std::string()
                                             : converter->Convert(pending);
  pending.clear();
  return output;
}

std::string ConverterStream::Finish(const char* input, size_t length) {
  if (length > 0) {
    pending.append(input, length);
  }
  return Finish();
}
