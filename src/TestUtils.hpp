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

#include <string>
#include "gtest/gtest.h"

namespace opencc {

using std::string;

#ifdef _MSC_VER
#define __func__ __FUNCTION__
#endif // ifdef _MSC_VER

#if defined(_MSC_VER) && _MSC_VER > 1310
// Visual C++ 2005 and later require the source files in UTF-8, and all strings
// to be encoded as wchar_t otherwise the strings will be converted into the
// local multibyte encoding and cause errors. To use a wchar_t as UTF-8, these
// strings then need to be convert back to UTF-8. This function is just a rough
// example of how to do this.
#include <Windows.h>
#define utf8(str) ConvertToUTF8(L##str)
string ConvertToUTF8(const wchar_t* pStr) {
  static char szBuf[1024];
  WideCharToMultiByte(CP_UTF8, 0, pStr, -1, szBuf, sizeof(szBuf), NULL, NULL);
  return szBuf;
}

#else // if defined(_MSC_VER) && _MSC_VER > 1310
// Visual C++ 2003 and gcc will use the string literals as is, so the files
// should be saved as UTF-8. gcc requires the files to not have a UTF-8 BOM.
#define utf8(str) string(str)
#endif // if defined(_MSC_VER) && _MSC_VER > 1310

#define EXPECT_VECTOR_EQ(expected, actual)                                     \
  {                                                                            \
    const auto& a1 = (expected);                                               \
    const auto& a2 = (actual);                                                 \
    EXPECT_EQ(a1.size(), a2.size());                                           \
    for (size_t i = 0; i < a1.size(); i++) {                                   \
      EXPECT_EQ(a1[i], a2[i]) << "Where i = " << i;                            \
    }                                                                          \
  }

} // namespace opencc
