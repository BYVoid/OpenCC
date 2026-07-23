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

#include "ConversionAmbiguities.hpp"

#include <cassert>
#include <cstring>
#include <list>
#include <unordered_map>
#include <utility>

#include "Common.hpp"
#include "ConfigBasedConverter.hpp"
#include "Conversion.hpp"
#include "ConversionChain.hpp"
#include "Converter.hpp"
#include "Dict.hpp"
#include "DictEntry.hpp"
#include "Optional.hpp"
#include "Segmentation.hpp"
#include "Segments.hpp"
#include "StreamWindow.hpp"
#include "UTF8Util.hpp"

namespace opencc {

namespace {

// One piece of the monotone piecewise alignment between a stage's input and
// its output: srcLen input bytes were rewritten into dstLen output bytes.
struct Run {
  size_t srcLen;
  size_t dstLen;
  bool ambiguous;
};

// Prefix match with PrefixMatch's group semantics, via the Dict interface.
// The virtual Dict::MatchPrefix() cannot be called directly on groups: a
// DictGroup constructed with DictGroupMatchPolicy::Union reports Union via
// GetMatchPolicy() -- which PrefixMatch honors by taking the longest match
// across children -- but its inherited MatchPrefix() still short-circuits;
// only the UnionDictGroup subclass overrides it.  Recursing on
// GetDictGroupItems()/GetMatchPolicy() applies the policy semantics
// uniformly, which also fixes plain Union-policy DictGroups nested inside
// other groups (where delegating to the parent's virtual MatchPrefix could
// not).
//
// The recursion is verified equivalent to both overrides (and to
// PrefixMatch's matcher tables): DictGroup::MatchPrefix returns the first
// child with any match, and UnionDictGroup::MatchPrefix takes the longest
// match across children with a strict > comparison so earlier children win
// ties -- exactly the two branches below.  Neither override merges entry
// values across children for prefix lookup, so NumValues() of the returned
// entry is the correct candidate count.  ConversionAmbiguitiesTest pins the
// longest-match and same-length-tie cases against Convert().
Optional<const DictEntry*> MatchPrefixLikeConversion(const Dict& dict,
                                                     const char* word,
                                                     size_t len) {
  const std::list<DictPtr>* items = dict.GetDictGroupItems();
  if (items == nullptr) {
    return dict.MatchPrefix(word, len);
  }
  if (dict.GetMatchPolicy() == DictGroupMatchPolicy::ShortCircuit) {
    for (const DictPtr& child : *items) {
      const Optional<const DictEntry*> match =
          MatchPrefixLikeConversion(*child, word, len);
      if (!match.IsNull()) {
        return match;
      }
    }
    return Optional<const DictEntry*>::Null();
  }
  Optional<const DictEntry*> best = Optional<const DictEntry*>::Null();
  for (const DictPtr& child : *items) {
    const Optional<const DictEntry*> match =
        MatchPrefixLikeConversion(*child, word, len);
    if (!match.IsNull() &&
        (best.IsNull() ||
         match.Get()->KeyLength() > best.Get()->KeyLength())) {
      best = match;
    }
  }
  return best;
}

// Replays Conversion::AppendConverted() for one dictionary over `in`, using
// the DictEntry-returning MatchPrefix() so the candidate count is visible.
// Appends the converted text to `out` and the alignment to `runs`.
//
// Runs must stay at match/character granularity: Compose() treats runs as
// atomic and widens groups until boundaries align, so coalescing adjacent
// unambiguous runs here would make a later-stage ambiguous match widen to
// the whole coalesced range (in real chains such as s2tw, where stage one
// rarely flags anything, that would stain an entire segment).
void WalkStage(const DictPtr& dict, std::string_view in, std::string* out,
               std::vector<Run>* runs) {
  const char* pstr = in.data();
  const char* const end = pstr + in.size();
  while (pstr < end) {
    const size_t remaining = static_cast<size_t>(end - pstr);
    const Optional<const DictEntry*> match =
        MatchPrefixLikeConversion(*dict, pstr, remaining);
    if (match.IsNull()) {
      size_t charLen =
          UTF8Util::NextIdeographicDescriptionSequenceLength(pstr, remaining);
      if (charLen == 0) {
        charLen = UTF8Util::NextCharLength(pstr);
      }
      if (charLen > remaining) {
        charLen = remaining;
      }
      out->append(pstr, charLen);
      runs->push_back(Run{charLen, charLen, false});
      pstr += charLen;
    } else {
      const DictEntry* entry = match.Get();
      size_t keyLen = entry->KeyLength();
      if (keyLen > remaining) {
        keyLen = remaining;
      }
      const std::string_view value = entry->GetDefaultView();
      out->append(value.data(), value.size());
      runs->push_back(Run{keyLen, value.size(), entry->NumValues() > 1});
      pstr += keyLen;
    }
  }
}

// Composes two adjacent piecewise alignments (a: T0 -> T1, b: T1 -> T2) into
// one (T0 -> T2).  Pieces are atomic, so where boundaries do not line up the
// composed piece conservatively covers the whole straddled range; a group is
// ambiguous if any constituent piece is.
std::vector<Run> Compose(const std::vector<Run>& a, const std::vector<Run>& b) {
  std::vector<Run> out;
  size_t ia = 0, ib = 0;
  while (ia < a.size() && ib < b.size()) {
    size_t srcLen = a[ia].srcLen;
    size_t aMid = a[ia].dstLen;
    bool ambiguous = a[ia].ambiguous;
    ia++;
    size_t bMid = b[ib].srcLen;
    size_t dstLen = b[ib].dstLen;
    ambiguous = ambiguous || b[ib].ambiguous;
    ib++;
    while (aMid != bMid) {
      if (aMid < bMid) {
        // The totals of a's dstLen and b's srcLen are equal (both measure
        // the intermediate text), so the lagging side always has runs left.
        assert(ia < a.size());
        srcLen += a[ia].srcLen;
        aMid += a[ia].dstLen;
        ambiguous = ambiguous || a[ia].ambiguous;
        ia++;
      } else {
        assert(ib < b.size());
        bMid += b[ib].srcLen;
        dstLen += b[ib].dstLen;
        ambiguous = ambiguous || b[ib].ambiguous;
        ib++;
      }
    }
    out.push_back(Run{srcLen, dstLen, ambiguous});
  }
  // Both sides measure the same intermediate text, so their totals match.
  // Every b-side run consumes at least one intermediate byte (WalkStage
  // advances by at least one byte per run), so b always exhausts with the
  // main loop.  The a-side, however, can trail with zero-width runs: an
  // a-side run's width on the middle coordinate is its dstLen, i.e. the
  // dictionary value's length, and an empty-value entry (constructible by
  // library users, e.g. StrSingleValueDictEntry("x", "")) matching at the
  // end of a segment yields dstLen == 0.  Fold such trailing runs into the
  // last group so the source side stays fully accounted for.
  assert(ib == b.size());
  for (; ia < a.size(); ia++) {
    assert(a[ia].dstLen == 0);
    if (out.empty()) {
      out.push_back(Run{0, 0, false});
    }
    out.back().srcLen += a[ia].srcLen;
    out.back().ambiguous = out.back().ambiguous || a[ia].ambiguous;
  }
  return out;
}

} // namespace

AnnotatedConversion ConvertWithAmbiguities(const Converter& converter,
                                           std::string_view text) {
  AnnotatedConversion result;
  const ConversionChainPtr chain = converter.GetConversionChain();
  if (chain == nullptr) {
    // No single conversion chain to walk (e.g. PipelineConverter); return the
    // plain conversion and flag the result as unanalyzed.
    result.output = converter.Convert(text);
    result.analyzed = false;
    return result;
  }

  // Mirror Convert() (and GetAllConversions()): run the normalization
  // pre-pass first, so sources are slices of the normalized input.
  std::string normalized;
  if (const auto* configBased =
          dynamic_cast<const ConfigBasedConverter*>(&converter)) {
    normalized = configBased->GetNormalizationConverter()->Convert(text);
    text = normalized;
  }

  std::unordered_map<std::string, size_t> sourceIndexes;
  // Each segment is walked through the whole chain before moving to the
  // next (segment-outer, stage-inner), while ConversionChain::Convert is
  // stage-outer, segment-inner.  The two orders are equivalent only
  // because every Conversion converts each segment independently and never
  // matches across segment boundaries; if the chain ever gains cross-
  // segment optimizations, this walk must be restructured to keep the
  // byte-identical-output contract with Converter::Convert().
  auto walkSegment = [&](std::string_view segmentView) {
    // Walk the segment through the chain, keeping the composed alignment
    // between the original segment and the current stage's output.
    std::vector<Run> aligned;
    std::string current(segmentView);
    bool firstStage = true;
    for (const ConversionPtr& conversion : chain->GetConversions()) {
      std::vector<Run> stageRuns;
      std::string next;
      next.reserve(current.size() + current.size() / 5);
      WalkStage(conversion->GetDict(), current, &next, &stageRuns);
      aligned = firstStage ? std::move(stageRuns) : Compose(aligned, stageRuns);
      firstStage = false;
      current.swap(next);
    }
    if (firstStage) {
      // Empty chain: the segment passes through unchanged.
      aligned.push_back(
          Run{segmentView.size(), segmentView.size(), false});
    }

    size_t srcOffset = 0;
    size_t dstOffset = result.output.size();
    for (const Run& run : aligned) {
      if (run.ambiguous) {
        std::string source(segmentView.substr(srcOffset, run.srcLen));
        const auto inserted =
            sourceIndexes.emplace(source, result.sources.size());
        if (inserted.second) {
          result.sources.push_back(std::move(source));
        }
        result.ambiguities.push_back(
            AmbiguousSpan{dstOffset, run.dstLen, inserted.first->second});
      }
      srcOffset += run.srcLen;
      dstOffset += run.dstLen;
    }
    result.output.append(current);
  };

  const SegmentationPtr segmentation = converter.GetSegmentation();
  if (segmentation == nullptr) {
    // Mirror SingleStageConverter::Convert's no-segmentation branch, which
    // converts the whole text as a single string_view without building a
    // Segments object -- embedded NUL bytes are preserved on both paths.
    walkSegment(text);
  } else {
    const SegmentsPtr segments = segmentation->Segment(text);
    for (const char* segment : *segments) {
      // strlen() mirrors Convert()'s segmented path, which also consumes
      // segments as NUL-terminated strings; input containing NUL bytes is
      // truncated identically on both paths, so offsets stay aligned.
      walkSegment(std::string_view(segment, std::strlen(segment)));
    }
  }
  return result;
}

class AmbiguityStream::Impl {
public:
  ConverterPtr converter;
  size_t maxKeepChars;
  std::string pending;
  std::unordered_map<std::string, size_t> sourceIndexes;
};

AmbiguityStream::AmbiguityStream(ConverterPtr converter, size_t maxKeepChars)
    : impl(new Impl) {
  impl->converter = std::move(converter);
  impl->maxKeepChars = maxKeepChars;
}

AmbiguityStream::~AmbiguityStream() {}

size_t AmbiguityStream::SourceCount() const {
  return impl->sourceIndexes.size();
}

// Converts one flushed window and rebases its chunk-local source indexes
// onto the stream-wide table, recording first-seen sources in newSources.
AmbiguityStream::Chunk
AmbiguityStream::ConvertWindow(std::string_view window) {
  Chunk chunk;
  AnnotatedConversion result =
      ConvertWithAmbiguities(*impl->converter, window);
  chunk.analyzed = result.analyzed;
  chunk.output = std::move(result.output);
  std::vector<size_t> globalIndex(result.sources.size());
  for (size_t i = 0; i < result.sources.size(); i++) {
    const auto inserted = impl->sourceIndexes.emplace(
        result.sources[i], impl->sourceIndexes.size());
    globalIndex[i] = inserted.first->second;
    if (inserted.second) {
      chunk.newSources.push_back(std::move(result.sources[i]));
    }
  }
  chunk.ambiguities = std::move(result.ambiguities);
  for (AmbiguousSpan& span : chunk.ambiguities) {
    span.sourceIndex = globalIndex[span.sourceIndex];
  }
  return chunk;
}

// Windowing below shares internal::FlushableByteCount with
// ConverterStream::ConvertChunk, so both wrappers flush on identical
// boundaries; the withheld tail guarantees no match (and hence no ambiguous
// span) ever straddles a flush boundary.
AmbiguityStream::Chunk AmbiguityStream::ConvertChunk(std::string_view input) {
  std::string& pending = impl->pending;
  if (!input.empty()) {
    pending.append(input);
  }
  const size_t flushable =
      internal::FlushableByteCount(pending, impl->maxKeepChars);
  if (flushable == 0) {
    return Chunk();
  }

  Chunk chunk =
      ConvertWindow(std::string_view(pending.data(), flushable));
  pending.erase(0, flushable);
  return chunk;
}

AmbiguityStream::Chunk AmbiguityStream::Finish(std::string_view input) {
  if (!input.empty()) {
    impl->pending.append(input);
  }
  return Finish();
}

AmbiguityStream::Chunk AmbiguityStream::Finish() {
  Chunk chunk;
  if (!impl->pending.empty()) {
    chunk = ConvertWindow(impl->pending);
    impl->pending.clear();
  }
  return chunk;
}

} // namespace opencc
