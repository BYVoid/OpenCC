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

#include <cstring>

#include "ConversionChain.hpp"
#include "Converter.hpp"
#include "Segments.hpp"

using namespace opencc;

std::string Converter::Convert(const std::string& text) const {
  const SegmentsPtr& segments = segmentation->Segment(text);
  const SegmentsPtr& converted = conversionChain->Convert(segments);
  return converted->ToString();
}

size_t Converter::Convert(const char* input, char* output) const {
  const std::string& converted = Convert(input);
  strcpy(output, converted.c_str());
  return converted.length();
}
