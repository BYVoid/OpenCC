/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 Carbo Kuo <byvoid@byvoid.com>
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

#include <cstddef>
#include <string>
#include <string_view>

#include "Common.hpp"
#include "ConversionInspection.hpp"
#include "Segmentation.hpp"

namespace opencc {
/**
 * Abstract base for all converters.
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT Converter {
public:
  virtual ~Converter() = default;

  virtual std::string Convert(std::string_view text) const = 0;

  virtual ConversionInspectionResult Inspect(const std::string& text) const = 0;

  virtual SegmentationPtr GetSegmentation() const = 0;

  virtual ConversionChainPtr GetConversionChain() const = 0;
};

class OPENCC_EXPORT ConverterStream {
public:
  explicit ConverterStream(ConverterPtr _converter, size_t _maxKeepChars = 16)
      : converter(_converter), maxKeepChars(_maxKeepChars) {}

  /**
   * Appends @p input to pending, then emits everything up to the last
   * @c maxKeepChars code points (extended if an incomplete IDS is detected).
   * The kept tail remains in pending so that a phrase or IDS spanning two
   * consecutive calls is not split across separate Convert() invocations.
   */
  std::string ConvertChunk(std::string_view input);

  /**
   * Appends @p input to pending and flushes everything at once.
   * @note Not equivalent to ConvertChunk(@p input) + Finish(): ConvertChunk
   *   applies the keepStart window and issues two Convert() calls, which
   *   breaks phrase or IDS matches that span the boundary.
   *   Use this overload for the final chunk when no more data is coming.
   */
  std::string Finish(std::string_view input);

  std::string Finish();

private:
  ConverterPtr converter;
  size_t maxKeepChars;
  std::string pending;
};
} // namespace opencc
