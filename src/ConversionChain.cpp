/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 BYVoid <byvoid@byvoid.com>
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

#include "ConversionChain.hpp"
#include "Segments.hpp"

using namespace opencc;

ConversionChain::ConversionChain(const list<ConversionPtr> _conversions)
    : conversions(_conversions) {}

SegmentsPtr ConversionChain::Convert(const SegmentsPtr& input) const {
  SegmentsPtr output = input;
  for (auto conversion : conversions) {
    output = conversion->Convert(output);
  }
  return output;
}
