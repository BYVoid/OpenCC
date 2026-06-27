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

#include "Converter.hpp"

namespace opencc {
/**
 * Single-stage converter: one segmentation pass followed by one conversion
 * chain.
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT SingleStageConverter : public Converter {
public:
  SingleStageConverter(SegmentationPtr segmentation,
                       ConversionChainPtr conversionChain)
      : segmentation(segmentation), conversionChain(conversionChain) {}

  std::string Convert(std::string_view text) const override;

  ConversionInspectionResult Inspect(const std::string& text) const override;

  SegmentationPtr GetSegmentation() const override { return segmentation; }

  ConversionChainPtr GetConversionChain() const override {
    return conversionChain;
  }

private:
  const SegmentationPtr segmentation;
  const ConversionChainPtr conversionChain;
};
} // namespace opencc
