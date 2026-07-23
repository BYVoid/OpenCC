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

#include "Converter.hpp"
#include "StreamWindow.hpp"

using namespace opencc;

std::string ConverterStream::ConvertChunk(std::string_view input) {
  if (!input.empty()) {
    pending.append(input);
  }
  const size_t flushable =
      internal::FlushableByteCount(pending, maxKeepChars);
  if (flushable == 0) {
    return std::string();
  }

  const std::string output =
      converter->Convert(std::string_view(pending.data(), flushable));
  pending.erase(0, flushable);
  return output;
}

std::string ConverterStream::Finish(std::string_view input) {
  if (!input.empty()) {
    pending.append(input);
  }
  return Finish();
}

std::string ConverterStream::Finish() {
  const std::string output =
      pending.empty() ? std::string()
                      : converter->Convert(std::string_view(pending));
  pending.clear();
  return output;
}

