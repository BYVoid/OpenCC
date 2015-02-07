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

namespace opencc {
/**
* Configuration loader
* @ingroup opencc_cpp_api
*/
class OPENCC_EXPORT Config {
public:
  Config();

  virtual ~Config();

  ConverterPtr NewFromString(const string& json, const string& configDirectory);

  ConverterPtr NewFromFile(const string& fileName);

private:
  void* internal;
};
}
