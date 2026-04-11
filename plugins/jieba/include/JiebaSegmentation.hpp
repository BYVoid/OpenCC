/*
 * Open Chinese Convert
 *
 * Copyright 2026 Frank Lin <github@linshuang.info>
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

namespace cppjieba {
class Jieba;
}

namespace opencc {

class JiebaSegmentation : public Segmentation {
public:
  JiebaSegmentation(const std::string& dictPath,
                    const std::string& modelPath,
                    const std::string& userDictPath = "");

  ~JiebaSegmentation() override;

  SegmentsPtr Segment(const std::string& text) const override;

private:
  std::unique_ptr<cppjieba::Jieba> jieba_;
};

} // namespace opencc
