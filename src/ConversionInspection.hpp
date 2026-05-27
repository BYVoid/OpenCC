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

#include <string>
#include <vector>

namespace opencc {

/**
 * Result of a single conversion stage during inspection.
 * @ingroup opencc_cpp_api
 */
struct ConversionInspectionStage {
  size_t index;
  std::vector<std::string> segments;
};

/**
 * Full inspection result for a single input text.
 * Contains the original input, initial segments after segmentation,
 * per-stage conversion results, and the final output.
 * @ingroup opencc_cpp_api
 */
struct ConversionInspectionResult {
  std::string input;
  std::vector<std::string> segments;
  std::vector<ConversionInspectionStage> stages;
  std::string output;
};

} // namespace opencc
