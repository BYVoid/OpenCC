/*
 * Open Chinese Convert
 *
 * Copyright 2010-2013 BYVoid <byvoid@byvoid.com>
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

#include "DictTestUtils.hpp"
#include "DictEntry.hpp"
#include "MaxMatchSegmentation.hpp"

using namespace Opencc;

int main(int argc, const char * argv[]) {
  // DictGroup
  auto dictGroup = DictTestUtils::CreateDictGroupForConversion();
  Optional<DictEntryPtr> entry;
  entry = dictGroup->MatchPrefix("Unknown");
  AssertTrue(entry.IsNull());
  
  DictEntryPtrVectorPtr matches = dictGroup->MatchAllPrefixes("干燥");
  AssertEquals(2, matches->size());
  AssertEquals("乾燥", matches->at(0)->GetDefault());
  AssertEquals("幹", matches->at(1)->GetDefault());
  
  // Segmentation
  auto segmentation = SegmentationPtr(new MaxMatchSegmentation(dictGroup));
  auto segments = segmentation->Segment("太后的头发干燥");
  AssertEquals(4, segments->size());
  AssertEquals("太后", segments->at(0)->key);
  AssertEquals("太后", segments->at(0)->GetDefault());
  AssertEquals("的", segments->at(1)->key);
  AssertEquals("的", segments->at(1)->GetDefault());
  AssertEquals("头发", segments->at(2)->key);
  AssertEquals("頭髮", segments->at(2)->GetDefault());
  AssertEquals("干燥", segments->at(3)->key);
  AssertEquals("乾燥", segments->at(3)->GetDefault());
  
  // Conversion
  auto conversion = ConversionPtr(new Conversion(segmentation));
  string converted = conversion->Convert("太后的头发干燥");
  AssertEquals("太后的頭髮乾燥", converted);
}
