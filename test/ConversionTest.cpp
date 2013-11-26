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

using namespace Opencc;

int main(int argc, const char * argv[]) {
  auto dictGroup = DictTestUtils::CreateDictGroupForConversion();
  Optional<DictEntry*> entry;
  entry = dictGroup->MatchPrefix("Unknown");
  assert(entry.IsNull());
  
  shared_ptr<vector<DictEntry*>> matches = dictGroup->MatchAllPrefixes("干燥");
  assert(matches->size() == 2);
  assert(matches->at(0)->GetDefault() == "乾燥");
  assert(matches->at(1)->GetDefault() == "幹");
}
