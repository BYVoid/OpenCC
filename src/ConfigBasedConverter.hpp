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

#include "Converter.hpp"

namespace opencc {
/**
 * A @c Converter produced by loading a JSON config that includes a
 * @c normalization pre-pass.
 *
 * Internally it runs @p normConverter then @p mainConverter, exactly like a
 * two-stage @c PipelineConverter.  The difference is that
 * @c GetConversionChain() and @c GetSegmentation() delegate to
 * @p mainConverter rather than returning @c nullptr, so that external
 * consumers (e.g. librime) that introspect the converter see the config's
 * primary conversion chain instead of a null pointer.
 */
class ConfigBasedConverter : public Converter {
public:
  ConfigBasedConverter(ConverterPtr normConverter, ConverterPtr mainConverter)
      : normConverter(std::move(normConverter)),
        mainConverter(std::move(mainConverter)) {}

  std::string Convert(std::string_view text) const override {
    return mainConverter->Convert(normConverter->Convert(text));
  }

  ConversionInspectionResult Inspect(std::string_view text) const override {
    ConversionInspectionResult result;
    result.input = text;
    ConversionInspectionResult normResult = normConverter->Inspect(text);
    ConversionInspectionResult mainResult =
        mainConverter->Inspect(normResult.output);
    result.pipelineStages = {std::move(normResult), std::move(mainResult)};
    result.output = result.pipelineStages.back().output;
    return result;
  }

  SegmentationPtr GetSegmentation() const override {
    return mainConverter->GetSegmentation();
  }

  ConversionChainPtr GetConversionChain() const override {
    return mainConverter->GetConversionChain();
  }

private:
  const ConverterPtr normConverter;
  const ConverterPtr mainConverter;
};
} // namespace opencc
