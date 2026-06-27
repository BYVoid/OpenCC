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

#include <string_view>

#include "Common.hpp"
#include "Segmentation.hpp"

namespace opencc {
class PrefixMatch;

/**
 * Single-dictionary phrase conversion.
 *
 * Applies prefix-match replacement using one dictionary (@c DictPtr) to an
 * already-segmented piece of text.  This is the lowest-level conversion
 * primitive: it knows nothing about segmentation and operates on a single
 * segment or a pre-built @c Segments object.
 *
 * Multiple @c Conversion objects are composed in a @c ConversionChain to
 * apply several dictionaries in order (e.g. phrase dictionary first, then
 * character dictionary).
 *
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT Conversion {
public:
  /** Constructs a Conversion backed by @p dict. */
  Conversion(DictPtr _dict);

  /**
   * Converts @p phrase using prefix-match replacement and returns the result.
   * @param phrase UTF-8 text; need not be null-terminated.
   */
  std::string Convert(std::string_view phrase) const;

  /**
   * Converts @p phrase using prefix-match replacement and returns the result.
   * @param phrase Null-terminated UTF-8 text.
   */
  std::string Convert(const char* phrase) const;

  /**
   * Converts @p phrase and appends the result to @p output.
   * Preferred in hot paths (e.g. ConversionChain) to avoid extra allocations.
   * @param phrase Null-terminated UTF-8 text.
   * @param output Destination buffer; content is appended, not replaced.
   */
  void AppendConverted(const char* phrase, std::string* output) const;

  /**
   * Converts every segment in @p input and returns a new @c Segments object.
   * Each segment is converted independently via Convert(const char*).
   */
  SegmentsPtr Convert(const SegmentsPtr& input) const;

  /** Returns the backing dictionary. */
  const DictPtr GetDict() const { return dict; }

private:
  const DictPtr dict;
  const std::shared_ptr<PrefixMatch> prefixMatch;
};
} // namespace opencc
