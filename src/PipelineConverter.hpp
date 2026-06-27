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

#include <utility>
#include <vector>

#include "Converter.hpp"

namespace opencc {
/**
 * A @c Converter that passes text through a sequence of @c Converter stages
 * in order, feeding the output of each stage as input to the next.
 *
 * This enables multi-step conversion schemes where different segmentation
 * strategies or dictionary sets are needed at each step.  For example, the
 * @c s2twp config first converts Simplified to Traditional (stage 1) and then
 * applies Taiwan-specific phrase substitution (stage 2).
 *
 * @note @c GetConversionChain() always returns @c nullptr because a pipeline
 * has no single chain.  @c GetSegmentation() returns the last stage's
 * segmentation as a best-effort representative.
 *
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT PipelineConverter : public Converter {
public:
  /** @param stages Ordered list of converters to apply in sequence. */
  explicit PipelineConverter(std::vector<ConverterPtr> stages)
      : stages(std::move(stages)) {}

  std::string Convert(std::string_view text) const override;

  /**
   * Returns an inspection result with @c input and @c output populated.
   * Per-stage segment detail is not available at the pipeline level.
   */
  ConversionInspectionResult Inspect(std::string_view text) const override;

  /** Returns the last stage's segmentation, or @c nullptr for an empty pipeline. */
  SegmentationPtr GetSegmentation() const override;

  /** Always returns @c nullptr; a pipeline has no single conversion chain. */
  ConversionChainPtr GetConversionChain() const override { return nullptr; }

private:
  const std::vector<ConverterPtr> stages;
};
} // namespace opencc
