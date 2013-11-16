/**
 * Open Chinese Convert
 *
 * Copyright 2013 BYVoid <byvoid@byvoid.com>
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

namespace Opencc {
  class UTF8Util {
  public:
    static void skipUtf8Bom(FILE* fp);

    static size_t nextCharLength(const char* str) {
      // FIXME next char
      return 1;
    }
    
    static const char* nextChar(const char* str) {
      return str + nextCharLength(str);
    }

    static const char* findNextInline(const char* str, const char ch) {
      while (!isLineEndingOrFileEnding(*str) && *str != ch) {
        str = nextChar(str);
      }
      return str;
    }
    
    static bool isLineEndingOrFileEnding(const char ch) {
      return ch == '\0' || ch == '\n' || ch == '\r';
    }
    
    static string fromSubstr(const char* str, size_t length) {
      string newStr(' ', length);
      for (size_t i = 0; i < length; i++, str++) {
        newStr[i] = *str;
      }
      return newStr;
    }
    
    static bool NotShorterThan(const char* str, size_t length) {
      while (length > 0) {
        if (*str == '\0') {
          return false;
        }
        length--;
        str++;
      }
      return true;
    }
    
    static string Truncate(const char* str, size_t maxLength) {
      string wordTrunc;
      if (NotShorterThan(str, maxLength)) {
        wordTrunc = fromSubstr(str, maxLength);
      } else {
        wordTrunc = str;
      }
      return wordTrunc;
    }
  };
}
