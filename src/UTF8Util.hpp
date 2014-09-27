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

namespace opencc {
  class OPENCC_EXPORT UTF8Util {
    public:
      static void SkipUtf8Bom(FILE* fp);

      static size_t NextCharLength(const char* str) {
        char ch = *str;
        if ((ch & 0x80) == 0x00) {
          return 1;
        } else if ((ch & 0xE0) == 0xC0) {
          return 2;
        } else if ((ch & 0xF0) == 0xE0) {
          return 3;
        } else if ((ch & 0xF8) == 0xF0) {
          return 4;
        } else if ((ch & 0xFC) == 0xF8) {
          return 5;
        } else if ((ch & 0xFE) == 0xFC) {
          return 6;
        }

        // Invalid UTF8 string or in the middle of a UTF8 char
        return 0;
      }

      static size_t PrevCharLength(const char* str) {
        for (size_t i = 1; i <= 6; i++) {
          str--;
          size_t length = NextCharLength(str);
          if (length == i) {
            return length;
          }
        }

        // Invalid UTF8 string or in the middle of a UTF8 char
        return 0;
      }

      static const char* NextChar(const char* str) {
        return str + NextCharLength(str);
      }

      static const char* PrevChar(const char* str) {
        return str - PrevCharLength(str);
      }

      static bool Validate(const char* str) {
        // TODO implement this
        return true;
      }

      static const char* FindNextInline(const char* str, const char ch) {
        while (!IsLineEndingOrFileEnding(*str) && *str != ch) {
          str = NextChar(str);
        }
        return str;
      }

      static bool IsLineEndingOrFileEnding(const char ch) {
        return ch == '\0' || ch == '\n' || ch == '\r';
      }

      static string FromSubstr(const char* str, size_t length) {
        string newStr;

        newStr.resize(length);
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

      static string TruncateUTF8(const char* str, size_t maxLength) {
        string wordTrunc;
        if (NotShorterThan(str, maxLength)) {
          size_t len = 0;
          const char* pStr = str;
          while (len < maxLength) {
            size_t nextLen = NextCharLength(pStr);
            pStr += nextLen;
            len += nextLen;
          }
          wordTrunc = FromSubstr(str, len);
        } else {
          wordTrunc = str;
        }
        return wordTrunc;
      }

      static void ReplaceAll(string& str, const char* from, const char* to) {
        string::size_type pos = 0;
        string::size_type fromLen = strlen(from);
        string::size_type toLen = strlen(to);
        while ((pos = str.find(from, pos)) != string::npos) {
          str.replace(pos, fromLen, to);
          pos += toLen;
        }
      }

      static string Join(StringVectorPtr strings, const string& separator) {
        std::ostringstream buffer;
        bool first = true;
        for (const auto& str : *strings) {
          if (!first) {
            buffer << ' ';
          }
          buffer << str;
          first = false;
        }
        return buffer.str();
      }

      static string Join(StringVectorPtr strings) {
        std::ostringstream buffer;
        for (const auto& str : *strings) {
          buffer << str;
        }
        return buffer.str();
      }
  };
}
