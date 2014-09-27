/**
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

#ifndef __OPENCC_H_
#define __OPENCC_H_

#ifdef __cplusplus

#include <string>
#include "Export.hpp"

namespace opencc {
  class OPENCC_EXPORT SimpleConverter {
  public:
    SimpleConverter(const std::string configFileName);
    ~SimpleConverter();
    std::string Convert(const std::string input) const;
  private:
    void* internalData;
  };
}

extern "C" {
#endif

typedef void* opencc_t;
  
opencc_t opencc_new(const char* configFileName);
void opencc_delete(opencc_t opencc);
char* opencc_convert(opencc_t opencc, const char* input);
void opencc_free_string(char* str);

#ifdef __cplusplus
}
#endif

#endif
