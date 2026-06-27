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

#include "SingleStageConverter.hpp"

#include "ConversionChain.hpp"
#include "ConversionInspection.hpp"
#include "Segments.hpp"

using namespace opencc;

std::string SingleStageConverter::Convert(std::string_view text) const {
  std::string converted;
  converted.reserve(text.length() + text.length() / 5);
  if (segmentation == nullptr) {
    // AppendConvertedSegment requires null-termination; copy once for this path
    const std::string owned(text);
    conversionChain->AppendConvertedSegment(owned.c_str(), &converted);
    return converted;
  }
  const SegmentsPtr& segments = segmentation->Segment(text);
  for (const char* segment : *segments) {
    conversionChain->AppendConvertedSegment(segment, &converted);
  }
  return converted;
}

ConversionInspectionResult
SingleStageConverter::Inspect(const std::string& text) const {
  ConversionInspectionResult result;
  result.input = text;

  SegmentsPtr initialSegments;
  if (segmentation == nullptr) {
    initialSegments.reset(new Segments);
    initialSegments->AddSegment(text);
  } else {
    initialSegments = segmentation->Segment(text);
  }
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
