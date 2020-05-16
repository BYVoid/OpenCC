/*
 * Open Chinese Convert
 *
 * Copyright 2015 Carbo Kuo <byvoid@byvoid.com>
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

#include <cstring>

#include "Common.hpp"
#include "UTF8Util.hpp"

namespace opencc {

namespace internal {

inline size_t FNVHash(const char* text, const size_t byteLength,
                      const size_t FNV_prime, const size_t FNV_offset_basis) {
  size_t hash = FNV_offset_basis;
  for (const char* pstr = text; pstr < text + byteLength; pstr++) {
    hash ^= *pstr;
    hash *= FNV_prime;
  }
  return hash;
}

template <int> size_t FNVHash(const char* text, const size_t byteLength);

template <>
inline size_t FNVHash<4>(const char* text, const size_t byteLength) {
  return FNVHash(text, byteLength, 16777619UL, 2166136261UL);
}

#if SIZE_MAX == 0xffffffffffffffff
template <>
inline size_t FNVHash<8>(const char* text, const size_t byteLength) {
  return FNVHash(text, byteLength, 1099511628211UL, 14695981039346656037UL);
}
#endif

} // namespace internal

template <typename LENGTH_TYPE> class UTF8StringSliceBase {
public:
  typedef LENGTH_TYPE LengthType;

  UTF8StringSliceBase(const char* _str)
      : str(_str), utf8Length(static_cast<LengthType>(UTF8Util::Length(_str))),
        byteLength(static_cast<LengthType>(strlen(_str))) {}

  UTF8StringSliceBase(const char* _str, const LengthType _utf8Length)
      : str(_str), utf8Length(_utf8Length) {
    CalculateByteLength();
  }

  UTF8StringSliceBase(const char* _str, const LengthType _utf8Length,
                      const LengthType _byteLength)
      : str(_str), utf8Length(_utf8Length), byteLength(_byteLength) {
    CalculateByteLength();
  }

  LengthType UTF8Length() const { return utf8Length; }

  LengthType ByteLength() const { return byteLength; }

  UTF8StringSliceBase Left(const LengthType numberOfCharacters) const {
    if (numberOfCharacters == UTF8Length()) {
      return *this;
    } else {
      return UTF8StringSliceBase(str, numberOfCharacters);
    }
  }

  UTF8StringSliceBase Right(const LengthType numberOfCharacters) const {
    if (numberOfCharacters == UTF8Length()) {
      return *this;
    } else {
      const char* pstr = str + byteLength;
      for (size_t i = 0; i < numberOfCharacters; i++) {
        pstr = UTF8Util::PrevChar(pstr);
      }
      return UTF8StringSliceBase(pstr, numberOfCharacters);
    }
  }

  UTF8StringSliceBase SubString(const LengthType offset,
                                const LengthType numberOfCharacters) const {
    if (offset == 0) {
      return Left(numberOfCharacters);
    } else {
      const char* pstr = str;
      for (size_t i = 0; i < offset; i++) {
        pstr = UTF8Util::NextChar(pstr);
      }
      return UTF8StringSliceBase(pstr, numberOfCharacters);
    }
  }

  std::string ToString() const { return std::string(str, str + byteLength); }

  const char* CString() const { return str; }

  LengthType CommonPrefixLength(const UTF8StringSliceBase& that) const {
    if (str == that.str) {
      return (std::min)(utf8Length, that.utf8Length);
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

  void MoveRight() {
    if (utf8Length > 0) {
      const size_t charLen = UTF8Util::NextCharLength(str);
      str += charLen;
      utf8Length--;
      byteLength -= charLen;
    }
  }

  void MoveLeft() {
    if (utf8Length > 0) {
      const size_t charLen = UTF8Util::PrevCharLength(str + byteLength);
      utf8Length--;
      byteLength -= charLen;
    }
  }

  int ReverseCompare(const UTF8StringSliceBase& that) const {
    const char* pstr1 = str + byteLength;
    const char* pstr2 = that.str + that.byteLength;
    const size_t length = (std::min)(utf8Length, that.utf8Length);
    for (size_t i = 0; i < length; i++) {
      const size_t charLen1 = UTF8Util::PrevCharLength(pstr1);
      const size_t charLen2 = UTF8Util::PrevCharLength(pstr2);
      pstr1 -= charLen1;
      pstr2 -= charLen2;
      const int cmp = strncmp(pstr1, pstr2, (std::min)(charLen1, charLen2));
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

  LengthType FindBytePosition(const UTF8StringSliceBase& pattern) const {
    return static_cast<LengthType>(
        ToString().find(pattern.str, 0, pattern.byteLength));
  }

  bool operator<(const UTF8StringSliceBase& that) const {
    return Compare(that) < 0;
  }

  bool operator>(const UTF8StringSliceBase& that) const {
    return Compare(that) > 0;
  }

  bool operator==(const UTF8StringSliceBase& that) const {
    return (str == that.str && utf8Length == that.utf8Length) ||
           Compare(that) == 0;
  }

  bool operator!=(const UTF8StringSliceBase& that) const {
    return !this->operator==(that);
  }

  class Hasher {
  public:
    size_t operator()(const UTF8StringSliceBase& text) const {
      return internal::FNVHash<sizeof(size_t)>(text.CString(),
                                               text.ByteLength());
    }
  };

private:
  inline int Compare(const UTF8StringSliceBase& that) const {
    int cmp = strncmp(str, that.str, (std::min)(byteLength, that.byteLength));
    if (cmp == 0) {
      if (utf8Length < that.utf8Length) {
        cmp = -1;
      } else if (utf8Length > that.utf8Length) {
        cmp = 1;
      } else {
        cmp = 0;
      }
    }
    return cmp;
  }

  void CalculateByteLength() {
    const char* pstr = str;
    for (size_t i = 0; i < utf8Length; i++) {
      pstr = UTF8Util::NextChar(pstr);
    }
    byteLength = static_cast<LengthType>(pstr - str);
  }

  const char* str;
  LengthType utf8Length;
  LengthType byteLength;
};

typedef UTF8StringSliceBase<size_t> UTF8StringSlice;

template <typename LENGTH_TYPE>
std::ostream& operator<<(::std::ostream& os,
                         const UTF8StringSliceBase<LENGTH_TYPE>& str) {
  return os << str.ToString();
}

} // namespace opencc
