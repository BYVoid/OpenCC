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

#include <list>

#include "Common.hpp"
#include "Conversion.hpp"

namespace opencc {
/**
 * An ordered sequence of @c Conversion objects applied to pre-segmented text.
 *
 * Each @c Conversion in the chain wraps one dictionary.  The chain applies
 * them in order: the output segments of conversion N become the input of
 * conversion N+1.  This enables priority-ordered substitution, such as
 * applying phrase dictionaries before character dictionaries so that
 * multi-character matches take precedence.
 *
 * A @c ConversionChain operates on @c Segments (output of @c Segmentation)
 * and has no knowledge of how those segments were produced.
 *
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT ConversionChain {
public:
  /** Constructs a chain from an ordered list of conversions. */
  ConversionChain(const std::list<ConversionPtr> _conversions);

  /**
   * Passes @p input through every conversion in the chain sequentially and
   * returns the final @c Segments.
   */
  SegmentsPtr Convert(const SegmentsPtr& input) const;

  /**
   * Converts the null-terminated @p segment through all conversions and
   * appends the result to @p output.  This is the hot-path entry point used
   * by @c SingleStageConverter to avoid intermediate string allocations.
   */
  void AppendConvertedSegment(const char* segment, std::string* output) const;

  /**
   * Converts @p input through the chain and records every intermediate
   * @c Segments after each conversion stage.
   * @return One entry per conversion in the chain, in order.
   */
  std::vector<SegmentsPtr> ConvertWithTrace(const SegmentsPtr& input) const;

  /** Returns the list of conversions in application order. */
  const std::list<ConversionPtr> GetConversions() const { return conversions; }

private:
  const std::list<ConversionPtr> conversions;
};
} // namespace opencc
