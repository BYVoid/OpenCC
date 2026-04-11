/*
 * Open Chinese Convert
 *
 * Copyright 2010-2026 Carbo Kuo and contributors
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

#if defined(_WIN32) || defined(_WIN64)

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <string>

namespace opencc {
namespace internal {

inline std::string Utf8FromWide(const std::wstring& wide) {
  if (wide.empty()) {
    return "";
  }
  const int required =
      WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr,
                          nullptr);
  if (required <= 1) {
    return "";
  }
  std::string utf8(static_cast<size_t>(required), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, utf8.data(), required,
                      nullptr, nullptr);
  utf8.resize(static_cast<size_t>(required - 1));
  return utf8;
}

inline std::wstring WideFromUtf8(const std::string& utf8) {
  if (utf8.empty()) {
    return L"";
  }
  const int required =
      MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
  if (required <= 1) {
    return L"";
  }
  std::wstring wide(static_cast<size_t>(required), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide.data(), required);
  wide.resize(static_cast<size_t>(required - 1));
  return wide;
}

} // namespace internal
} // namespace opencc

#endif // defined(_WIN32) || defined(_WIN64)
