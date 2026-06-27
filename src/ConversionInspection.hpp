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
 * Full inspection result returned by Converter::Inspect().
 *
 * The fields that are populated depend on the concrete Converter type:
 *
 * - @b SingleStageConverter fills @c segments (initial segmentation),
 *   @c stages (per-Conversion trace through the ConversionChain), and
 *   @c output.  @c pipelineStages is empty.
 *
 * - @b PipelineConverter fills @c pipelineStages (one entry per child
 *   Converter, each with its own fully populated result) and @c output.
 *   @c segments and @c stages are empty at the pipeline level.
 *
 * In both cases @c input and @c output are always populated.
 *
 * @ingroup opencc_cpp_api
 */
struct ConversionInspectionResult {
  /** The original input text passed to Inspect(). */
  std::string input;

  /** Initial segmentation produced by this converter's Segmentation.
   *  Non-empty only for SingleStageConverter. */
  std::vector<std::string> segments;

  /** Per-dictionary-stage trace through this converter's ConversionChain.
   *  Non-empty only for SingleStageConverter. */
  std::vector<ConversionInspectionStage> stages;

  /** Per-child-converter inspection results for a PipelineConverter.
   *  Each entry is the full result of the corresponding pipeline stage;
   *  the structure is naturally recursive for nested pipelines.
   *  Empty for SingleStageConverter. */
  std::vector<ConversionInspectionResult> pipelineStages;

  /** The final converted output of this converter. Always populated. */
  std::string output;
};

} // namespace opencc
