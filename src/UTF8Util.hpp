/*
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

#ifdef _MSC_VER
#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX
#endif // _MSC_VER

#include "Common.hpp"

namespace opencc {
/**
* UTF8 string utilities
* @ingroup opencc_cpp_api
*/
class OPENCC_EXPORT UTF8Util {
public:
  /**
  * Detect UTF8 BOM and skip it.
  */
  static void SkipUtf8Bom(FILE* fp);

  /**
  * Returns the length in byte for the next UTF8 character.
  * On error returns 0.
  */
  static size_t NextCharLengthNoException(const char* str) {
    char ch = *str;
    if ((ch & 0xF0) == 0xE0) {
      return 3;
    } else if ((ch & 0x80) == 0x00) {
      return 1;
    } else if ((ch & 0xE0) == 0xC0) {
      return 2;
    } else if ((ch & 0xF8) == 0xF0) {
      return 4;
    } else if ((ch & 0xFC) == 0xF8) {
      return 5;
    } else if ((ch & 0xFE) == 0xFC) {
      return 6;
    }
    return 0;
  }

  /**
   * Returns the length in byte for the next UTF8 character.
   */
  static size_t NextCharLength(const char* str) {
    size_t length = NextCharLengthNoException(str);
    if (length == 0) {
      throw InvalidUTF8(str);
    }
    return length;
  }

  /**
  * Returns the length in byte for the previous UTF8 character.
  */
  static size_t PrevCharLength(const char* str) {
    {
      const size_t length = NextCharLengthNoException(str - 3);
      if (length == 3) {
        return length;
      }
    }
    {
      const size_t length = NextCharLengthNoException(str - 1);
      if (length == 1) {
        return length;
      }
    }
    {
      const size_t length = NextCharLengthNoException(str - 2);
      if (length == 2) {
        return length;
      }
    }
    for (size_t i = 4; i <= 6; i++) {
      const size_t length = NextCharLengthNoException(str - i);
      if (length == i) {
        return length;
      }
    }
    throw InvalidUTF8(str);
  }

  /**
  * Returns the char* pointer over the next UTF8 character.
  */
  static const char* NextChar(const char* str) {
    return str + NextCharLength(str);
  }

  /**
  * Move the char* pointer before the previous UTF8 character.
  */
  static const char* PrevChar(const char* str) {
    return str - PrevCharLength(str);
  }

  /**
   * Returns the UTF8 length of a valid UTF8 string.
   */
  static size_t Length(const char* str) {
    size_t length = 0;
    while (*str != '\0') {
      str = NextChar(str);
      length++;
    }
    return length;
  }

  /**
  * Finds a character in the same line.
  * @param str The text to be searched in.
  * @param ch  The character to find.
  * @return    The pointer that points to the found chacter in str or EOL/EOF.
  */
  static const char* FindNextInline(const char* str, const char ch) {
    while (!IsLineEndingOrFileEnding(*str) && *str != ch) {
      str = NextChar(str);
    }
    return str;
  }

  /**
  * Returns ture if the character is a line ending or end of file.
  */
  static bool IsLineEndingOrFileEnding(const char ch) {
    return ch == '\0' || ch == '\n' || ch == '\r';
  }

  /**
  * Copies a substring with given length to a new std::string.
  */
  static string FromSubstr(const char* str, size_t length) {
    string newStr;
    newStr.resize(length);
    strncpy(const_cast<char*>(newStr.c_str()), str, length);
    return newStr;
  }

  /**
  * Returns true if the given string is longer or as long as the given length.
  */
  static bool NotShorterThan(const char* str, size_t byteLength) {
    while (byteLength > 0) {
      if (*str == '\0') {
        return false;
      }
      byteLength--;
      str++;
    }
    return true;
  }

  /**
  * Truncates a string with a maximal length in byte.
  * No UTF8 character will be broken.
  */
  static string TruncateUTF8(const char* str, size_t maxByteLength) {
    string wordTrunc;
    if (NotShorterThan(str, maxByteLength)) {
      size_t len = 0;
      const char* pStr = str;
      for (;;) {
        const size_t charLength = NextCharLength(pStr);
        if (len + charLength > maxByteLength) {
          break;
        }
        pStr += charLength;
        len += charLength;
      }
      wordTrunc = FromSubstr(str, len);
    } else {
      wordTrunc = str;
    }
    return wordTrunc;
  }

  /**
  * Replaces all patterns in a string in place.
  */
  static void ReplaceAll(string& str, const char* from, const char* to) {
    string::size_type pos = 0;
    string::size_type fromLen = strlen(from);
    string::size_type toLen = strlen(to);
    while ((pos = str.find(from, pos)) != string::npos) {
      str.replace(pos, fromLen, to);
      pos += toLen;
    }
  }

  /**
  * Joins a string vector in to a string with a separator.
  */
  static string Join(const vector<string>& strings, const string& separator) {
    std::ostringstream buffer;
    bool first = true;
    for (const auto& str : strings) {
      if (!first) {
        buffer << separator;
      }
      buffer << str;
      first = false;
    }
    return buffer.str();
  }

  /**
  * Joins a string vector in to a string.
  */
  static string Join(const vector<string>& strings) {
    std::ostringstream buffer;
    for (const auto& str : strings) {
      buffer << str;
    }
    return buffer.str();
  }

  static void GetByteMap(const char* str, const size_t utf8Length,
                         vector<size_t>* byteMap) {
    if (byteMap->size() < utf8Length) {
      byteMap->resize(utf8Length);
    }
    const char* pstr = str;
    for (size_t i = 0; i < utf8Length; i++) {
      (*byteMap)[i] = pstr - str;
      pstr = NextChar(pstr);
    }
  }

#ifdef _MSC_VER
  static std::wstring GetPlatformString(const std::string& str) {
    return U8ToU16(str);
  }
#else
  static std::string GetPlatformString(const std::string& str) {
    return str;
  }
#endif // _MSC_VER


#ifdef _MSC_VER
  static std::string U16ToU8(const std::wstring& wstr) {
    std::string ret;
    int length = static_cast<int>(wstr.length());
    int convcnt = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), length, NULL, 0, NULL, NULL);
    if (convcnt > 0) {
      ret.resize(convcnt);
      WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), length, &ret[0], convcnt, NULL, NULL);
    }
    return ret;
  }

  static std::wstring U8ToU16(const std::string& str) {
    std::wstring ret;
    int length = static_cast<int>(str.length());
    int convcnt = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), length, NULL, 0);
    if (convcnt > 0) {
      ret.resize(convcnt);
      MultiByteToWideChar(CP_UTF8, 0, str.c_str(), length, &ret[0], convcnt);
    }
    return ret;
  }
#endif // _MSC_VER
};
}
