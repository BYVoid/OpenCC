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

#include <string>
#include <string_view>
#include <vector>

#include "Export.hpp"

namespace opencc {

class Converter;

/**
 * Enumerates every candidate form of a single word produced by running it
 * through @p converter's conversion chain.
 *
 * Unlike Converter::Convert(), which segments the whole text and yields exactly
 * one output string, this walks @p word through each dictionary in the chain
 * and keeps @b all branch values.  For example, @c s2t expands @c 里 to both
 * @c 里 and @c 裏, then @c t2tw passes @c 里 through unchanged and converts
 * @c 裏 to @c 裡.  This mirrors the behaviour input-method engines (e.g.
 * librime's @c ConvertWord) need to offer users every plausible conversion of a
 * single word.
 *
 * Matching a word against a dictionary:
 *  - On an exact match, every value of the entry becomes a candidate.
 *  - Otherwise the word is converted greedily by longest-prefix, taking each
 *    matched prefix's default value, so a partially convertible word still
 *    flows through the remaining dictionaries in the chain.
 *
 * @param converter Source of the conversion chain.  A converter without a
 *   single chain (e.g. @c PipelineConverter, whose GetConversionChain()
 *   returns @c nullptr) yields an empty result.
 * @param word UTF-8 text of a single word; need not be null-terminated.
 * @return Candidate forms in discovery order with duplicates removed.  Empty
 *   when no dictionary in the chain contains @p word, matching librime's
 *   convention of reporting "not found".
 *
 * @note Internal, unstable API.  This header is intentionally kept out of the
 *   installed public header set and is not part of the OPENCC_ABI_VERSION
 *   contract.  It backs the experimental Converter::GetConversionCandidates()
 *   member, which is compiled only when OPENCC_ENABLE_UNSTABLE_API is defined.
 */
OPENCC_EXPORT std::vector<std::string>
GetAllConversions(const Converter& converter, std::string_view word);

} // namespace opencc
