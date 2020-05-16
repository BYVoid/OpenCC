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

#include "Common.hpp"
#include "DictGroup.hpp"
#include "Segmentation.hpp"

namespace opencc {
/**
 * Implementation of maximal match segmentation
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT MaxMatchSegmentation : public Segmentation {
public:
  MaxMatchSegmentation(const DictPtr _dict) : dict(_dict) {}

  virtual ~MaxMatchSegmentation() {}

  virtual SegmentsPtr Segment(const std::string& text) const;

  const DictPtr GetDict() const { return dict; }

private:
  const DictPtr dict;
};
} // namespace opencc
