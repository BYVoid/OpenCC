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

#pragma once

#include "Common.hpp"
#include "Segmentation.hpp"

namespace opencc {
/**
* Controller of segmentation and conversion
* @ingroup opencc_cpp_api
*/
class OPENCC_EXPORT Converter {
public:
  Converter(const string& _name, SegmentationPtr _segmentation,
            ConversionChainPtr _conversionChain)
      : name(_name), segmentation(_segmentation),
        conversionChain(_conversionChain) {}

  string Convert(const string& text) const;

  size_t Convert(const char* input, char* output) const;

  const SegmentationPtr GetSegmentation() const { return segmentation; }

  const ConversionChainPtr GetConversionChain() const {
    return conversionChain;
  }

private:
  const string name;
  const SegmentationPtr segmentation;
  const ConversionChainPtr conversionChain;
};
}
