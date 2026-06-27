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

#include "PipelineConverter.hpp"

using namespace opencc;

std::string PipelineConverter::Convert(std::string_view text) const {
  if (stages.empty()) {
    return std::string(text);
  }
  auto it = stages.begin();
  std::string result = (*it)->Convert(text);
  for (++it; it != stages.end(); ++it) {
    result = (*it)->Convert(result);
  }
  return result;
}

ConversionInspectionResult
PipelineConverter::Inspect(std::string_view text) const {
  ConversionInspectionResult result;
  result.input = text;

  std::string current(text);
  result.pipelineStages.reserve(stages.size());
  for (const ConverterPtr& stage : stages) {
    ConversionInspectionResult stageResult = stage->Inspect(current);
    current = stageResult.output;
    result.pipelineStages.push_back(std::move(stageResult));
  }

  result.output = current;
  return result;
}

SegmentationPtr PipelineConverter::GetSegmentation() const {
  return stages.empty() ? nullptr : stages.back()->GetSegmentation();
}
