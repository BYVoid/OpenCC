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
 * Abstract base for full-text converters.
 *
 * A @c Converter accepts a complete UTF-8 string and returns the converted
 * result.  Internally it coordinates @c Segmentation (splitting text into
 * words) and a @c ConversionChain (applying one or more dictionaries to each
 * segment).  Concrete implementations:
 *
 *  - @c SingleStageConverter — one segmentation pass + one @c ConversionChain.
 *  - @c PipelineConverter — passes text through a sequence of @c Converter
 *    stages in order, each with its own segmentation and chain.
 *
 * Config-backed converters may additionally expose the main conversion chain
 * via @c GetConversionChain() even when an internal normalization pre-pass is
 * present.
 *
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT Converter {
public:
  virtual ~Converter() = default;

  /**
   * Converts @p text and returns the result.
   * @param text UTF-8 input; need not be null-terminated.
   */
  virtual std::string Convert(std::string_view text) const = 0;

  /**
   * Converts @p text and returns a detailed inspection result that includes
   * the initial segmentation, per-stage intermediate segments, and final
   * output.  Intended for debugging and tooling rather than production
   * conversion.
   */
  virtual ConversionInspectionResult Inspect(std::string_view text) const = 0;

  /**
   * Returns the segmentation used by this converter, or @c nullptr if the
   * converter has no single segmentation (e.g. @c PipelineConverter).
   */
  virtual SegmentationPtr GetSegmentation() const = 0;

  /**
   * Returns the conversion chain used by this converter, or @c nullptr if the
   * converter has no single chain (e.g. @c PipelineConverter).
   */
  virtual ConversionChainPtr GetConversionChain() const = 0;
};

/**
 * Default number of Unicode code points a streaming converter retains at
 * the end of its pending buffer after each flush, so that a phrase or IDS
 * straddling two consecutive chunks is not split across separate
 * conversions.  Shared by every streaming wrapper (e.g. ConverterStream
 * and the internal AmbiguityStream) so their windowing stays identical.
 */
inline constexpr size_t kDefaultStreamKeepChars = 16;

class OPENCC_EXPORT ConverterStream {
public:
  /**
   * @param converter   The converter applied to each flushed chunk.
   * @param maxKeepChars  Number of Unicode code points (not bytes) to retain
   *   at the end of the pending buffer after each ConvertChunk() call, so
   *   that a phrase or IDS straddling two consecutive chunks is not split
   *   across separate Convert() invocations.
   */
  explicit ConverterStream(ConverterPtr converter,
                           size_t maxKeepChars = kDefaultStreamKeepChars)
      : converter(converter), maxKeepChars(maxKeepChars) {}

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
