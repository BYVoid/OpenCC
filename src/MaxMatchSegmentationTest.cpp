/*
 * Open Chinese Convert
 *
 * Copyright 2015 BYVoid <byvoid@byvoid.com>
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

#include "DictGroupTestBase.hpp"
#include "MaxMatchSegmentation.hpp"

namespace opencc {

class MaxMatchSegmentationTest : public DictGroupTestBase {
protected:
  MaxMatchSegmentationTest() {}

  virtual void SetUp() {
    dict = CreateDictGroupForConversion();
    segmenter = SegmentationPtr(new MaxMatchSegmentation(dict));
  }

  DictPtr dict;
  SegmentationPtr segmenter;
};

TEST_F(MaxMatchSegmentationTest, Segment) {
  const auto& segments = segmenter->Segment(utf8("太后的头发干燥"));
  EXPECT_EQ(4, segments->Length());
  EXPECT_EQ(utf8("太后"), string(segments->At(0)));
  EXPECT_EQ(utf8("的"), string(segments->At(1)));
  EXPECT_EQ(utf8("头发"), string(segments->At(2)));
  EXPECT_EQ(utf8("干燥"), string(segments->At(3)));
}

} // namespace opencc
