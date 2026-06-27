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
 * Pipeline converter: passes text through a sequence of converters in order.
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT PipelineConverter : public Converter {
public:
  explicit PipelineConverter(std::vector<ConverterPtr> stages)
      : stages(std::move(stages)) {}

  std::string Convert(std::string_view text) const override;

  /** Returns an object with input/output filled and no stage detail. */
  ConversionInspectionResult Inspect(const std::string& text) const override;

  /** Returns the last stage's segmentation, or nullptr for an empty pipeline. */
  SegmentationPtr GetSegmentation() const override;

  /** Returns nullptr; a pipeline has no single conversion chain. */
  ConversionChainPtr GetConversionChain() const override { return nullptr; }

private:
  const std::vector<ConverterPtr> stages;
};
} // namespace opencc
