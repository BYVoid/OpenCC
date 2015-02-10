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

#include "Common.hpp"
#include "UTF8Util.hpp"

namespace opencc {

class UTF8StringSlice {
public:
  UTF8StringSlice(const char* _str)
      : str(_str), utf8Length(UTF8Util::Length(_str)),
        byteLength(strlen(_str)) {}

  UTF8StringSlice(const char* _str, size_t _utf8Length)
      : str(_str), utf8Length(_utf8Length) {
    CalculateByteLength();
  }

  size_t UTF8Length() const { return utf8Length; }

  size_t ByteLength() const { return byteLength; }

  UTF8StringSlice Left(const size_t utf8Length) const;

  UTF8StringSlice Right(const size_t utf8Length) const;

  UTF8StringSlice SubString(const size_t offset, const size_t utf8Length) const;

  string ToString() const { return string(str, str + byteLength); }

  size_t CommonPrefixLength(const UTF8StringSlice& that) const;

  void MoveRight();

  void MoveLeft();

  int ReverseCompare(const UTF8StringSlice& that) const;

  size_t FindBytePosition(const UTF8StringSlice& pattern) const;

  bool operator<(const UTF8StringSlice& that) const {
    return Compare(that) < 0;
  }

  bool operator>(const UTF8StringSlice& that) const {
    return Compare(that) > 0;
  }

  bool operator==(const UTF8StringSlice& that) const {
    return (str == that.str && utf8Length == that.utf8Length) ||
           Compare(that) == 0;
  }

  bool operator!=(const UTF8StringSlice& that) const {
    return !this->operator==(that);
  }

  class Hasher {
  public:
    size_t operator()(const UTF8StringSlice& text) const;
  };

private:
  inline int Compare(const UTF8StringSlice& that) const {
    int cmp = strncmp(str, that.str, std::min(byteLength, that.byteLength));
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

  void CalculateByteLength();

  const char* str;
  size_t utf8Length;
  size_t byteLength;
};

std::ostream& operator<<(::std::ostream& os, const UTF8StringSlice& str);

} // namespace opencc
