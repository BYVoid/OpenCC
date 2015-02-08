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

#include "UTF8StringSlice.hpp"

namespace opencc {

UTF8StringSlice UTF8StringSlice::Left(const size_t utf8Length) const {
  if (utf8Length == UTF8Length()) {
    return *this;
  } else {
    return UTF8StringSlice(str, utf8Length);
  }
}

UTF8StringSlice UTF8StringSlice::Right(const size_t utf8Length) const {
  if (utf8Length == UTF8Length()) {
    return *this;
  } else {
    const char* pstr = str + byteLength;
    for (size_t i = 0; i < utf8Length; i++) {
      pstr = UTF8Util::PrevChar(pstr);
    }
    return UTF8StringSlice(pstr, utf8Length);
  }
}

UTF8StringSlice UTF8StringSlice::SubString(const size_t offset,
                                           const size_t utf8Length) const {
  if (offset == 0) {
    return Left(utf8Length);
  } else {
    const char* pstr = str;
    for (size_t i = 0; i < offset; i++) {
      pstr = UTF8Util::NextChar(pstr);
    }
    return UTF8StringSlice(pstr, utf8Length);
  }
}

size_t UTF8StringSlice::CommonPrefixLength(const UTF8StringSlice& that) const {
  if (str == that.str) {
    return std::min(utf8Length, that.utf8Length);
  } else {
    const char* pstr1 = str;
    const char* pstr2 = that.str;
    for (size_t length = 0; length < utf8Length && length < that.utf8Length;
         length++) {
      size_t charLen1 = UTF8Util::NextCharLength(pstr1);
      size_t charLen2 = UTF8Util::NextCharLength(pstr2);
      if (charLen1 != charLen2 || strncmp(pstr1, pstr2, charLen1) != 0) {
        return length;
      }
      pstr1 += charLen1;
      pstr2 += charLen2;
    }
    return 0;
  }
}

void UTF8StringSlice::MoveRight() {
  if (utf8Length > 0) {
    const size_t charLen = UTF8Util::NextCharLength(str);
    str += charLen;
    utf8Length--;
    byteLength -= charLen;
  }
}

void UTF8StringSlice::MoveLeft() {
  if (utf8Length > 0) {
    const size_t charLen = UTF8Util::PrevCharLength(str + byteLength);
    utf8Length--;
    byteLength -= charLen;
  }
}

int UTF8StringSlice::ReverseCompare(const UTF8StringSlice& that) const {
  const char* pstr1 = str + byteLength;
  const char* pstr2 = that.str + that.byteLength;
  const size_t length = std::min(utf8Length, that.utf8Length);
  for (size_t i = 0; i < length; i++) {
    const size_t charLen1 = UTF8Util::PrevCharLength(pstr1);
    const size_t charLen2 = UTF8Util::PrevCharLength(pstr2);
    pstr1 -= charLen1;
    pstr2 -= charLen2;
    const int cmp = strncmp(pstr1, pstr2, std::min(charLen1, charLen2));
    if (cmp < 0) {
      return -1;
    } else if (cmp > 0) {
      return 1;
    } else if (charLen1 < charLen2) {
      return -1;
    } else if (charLen1 > charLen2) {
      return 1;
    }
  }
  if (utf8Length < that.utf8Length) {
    return -1;
  } else if (utf8Length > that.utf8Length) {
    return 1;
  } else {
    return 0;
  }
}

size_t UTF8StringSlice::FindBytePosition(const UTF8StringSlice& pattern) const {
  const size_t pos = ToString().find(pattern.ToString());
  if (pos != string::npos) {
    return pos;
  } else {
    return static_cast<size_t>(-1);
  }
}

void UTF8StringSlice::CalculateByteLength() {
  const char* pstr = str;
  for (size_t i = 0; i < utf8Length; i++) {
    pstr = UTF8Util::NextChar(pstr);
  }
  byteLength = pstr - str;
}

} // namespace opencc
