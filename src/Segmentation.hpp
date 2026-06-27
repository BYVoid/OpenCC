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

#include <string>
#include <string_view>

#include "Common.hpp"

namespace opencc {
/**
 * Abstract base for text segmentation.
 *
 * Splits a UTF-8 string into an ordered list of segments. Each segment is
 * either a dictionary-matched word or a single unmatched code point / IDS
 * sequence.  The primary virtual is Segment(std::string_view); the
 * @c const @c char* and @c const @c std::string& overloads are non-virtual
 * convenience adapters that forward to it.
 *
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT Segmentation {
public:
  virtual ~Segmentation() {}

  /**
   * Splits @p text into segments and returns them.
   * This is the primary override point for subclasses.
   */
  virtual SegmentsPtr Segment(std::string_view text) const = 0;

  /** Convenience overload for null-terminated C strings. */
  SegmentsPtr Segment(const char* text) const;

  /** Convenience overload for std::string. */
  SegmentsPtr Segment(const std::string& str) const;
};
} // namespace opencc
