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

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Common.hpp"
#include "Converter.hpp"
#include "Export.hpp"

namespace opencc {

/**
 * One span of the converted output whose source maps to more than one
 * candidate (a one-to-many dictionary entry was applied), e.g. 文丑 in s2t,
 * where 丑 maps to both 醜 and 丑 and either 文醜 (the Three Kingdoms
 * general) or 文丑 (the opera role) may be intended.
 *
 * Offsets and lengths are UTF-8 byte counts into
 * AnnotatedConversion::output.  The output bytes
 * [outputOffset, outputOffset + outputLength) hold the default candidate.
 * The input slice that produced this span is
 * AnnotatedConversion::sources[sourceIndex]; passing it to
 * GetAllConversions() enumerates every candidate.
 */
struct AmbiguousSpan {
  size_t outputOffset;
  size_t outputLength;
  size_t sourceIndex;
};

/**
 * Result of ConvertWithAmbiguities(): the converted text plus the spans
 * where the conversion is one-to-many.
 *
 * @c output is byte-identical to Converter::Convert() on the same input.
 * @c sources holds the deduplicated input slices that produced ambiguous
 * spans, in order of first appearance; input text repeats heavily in
 * practice, so spans reference sources by index instead of embedding a
 * copy (and callers can memoize GetAllConversions() per unique source).
 * @c ambiguities is sorted by outputOffset and spans do not overlap.
 *
 * For a converter with a normalization pre-pass (e.g. s2t.json), sources
 * are slices of the normalized input, mirroring GetAllConversions().
 */
struct AnnotatedConversion {
  std::string output;
  std::vector<std::string> sources;
  std::vector<AmbiguousSpan> ambiguities;
  /** False when the converter could not be analyzed (no single conversion
   *  chain, e.g. PipelineConverter): output is still the plain conversion
   *  result, but empty ambiguities then means "unknown", not "none". */
  bool analyzed = true;
};

/**
 * Converts @p text exactly like @p converter->Convert() and additionally
 * reports every output span whose dictionary match is one-to-many.
 *
 * A span is reported when the applied entry has more than one value at any
 * stage of the conversion chain.  The check is per-entry, so a span is a
 * *potential* ambiguity: candidates that later stages map back to the same
 * final form are not filtered out.  Callers resolve candidates on demand
 * with GetAllConversions(converter, sources[span.sourceIndex]).
 *
 * When a later-stage match straddles earlier match boundaries the reported
 * span conservatively covers the whole straddled range.
 *
 * A converter without a single conversion chain (e.g. PipelineConverter,
 * whose GetConversionChain() returns nullptr) yields the plain conversion
 * result with no ambiguity information.
 *
 * @note Internal, unstable API with no compatibility guarantee.  This
 *   header is not installed by CMake and the function is not part of the
 *   OPENCC_ABI_VERSION contract; like ConversionCandidates.hpp the symbol
 *   is OPENCC_EXPORT only so unit tests can link against a shared
 *   libopencc on Windows.
 */
OPENCC_EXPORT AnnotatedConversion
ConvertWithAmbiguities(const Converter& converter, std::string_view text);

/**
 * Default keep-tail window for AmbiguityStream, in Unicode code points.
 * Must equal ConverterStream's default maxKeepChars (Converter.hpp) so the
 * two wrappers flush on identical boundaries; the constant lives in this
 * private header instead of the installed one to keep this feature free of
 * installed-header changes.  ConversionAmbiguitiesTest pins the two
 * defaults together behaviorally (identical per-chunk flush output), so a
 * drift fails tests instead of silently desynchronizing the streams.
 */
inline constexpr size_t kAmbiguityStreamDefaultKeepChars = 16;

/**
 * Streaming variant of ConvertWithAmbiguities() with bounded memory.
 *
 * Mirrors ConverterStream's windowing (a tail of @p maxKeepChars code
 * points is withheld from each flush so matches never straddle a window
 * boundary) but reports ambiguous spans per flushed chunk.  Producer-side
 * state is the pending tail plus one global source-deduplication table
 * whose size is bounded by the dictionaries' one-to-many entries, not by
 * the input length, enabling define-on-first-use record emission:
 * Chunk::newSources lists the sources first seen in this chunk (in global
 * index order), and span sourceIndex values are global stream-wide
 * indexes.  Offsets in Chunk::ambiguities are relative to Chunk::output.
 *
 * @note Internal, unstable API; see ConvertWithAmbiguities().
 */
class OPENCC_EXPORT AmbiguityStream {
public:
  struct Chunk {
    std::string output;
    std::vector<AmbiguousSpan> ambiguities;
    std::vector<std::string> newSources;
  };

  explicit AmbiguityStream(ConverterPtr converter,
                           size_t maxKeepChars = kAmbiguityStreamDefaultKeepChars);
  ~AmbiguityStream();

  /** Appends @p input and flushes everything but the kept tail. */
  Chunk ConvertChunk(std::string_view input);

  /** Appends @p input and flushes everything, ending the stream. */
  Chunk Finish(std::string_view input);

  /** Flushes everything, ending the stream. */
  Chunk Finish();

  /** Number of distinct sources defined so far. */
  size_t SourceCount() const;

private:
  Chunk ConvertWindow(std::string_view window);

  class Impl;
  std::unique_ptr<Impl> impl;
};

} // namespace opencc
