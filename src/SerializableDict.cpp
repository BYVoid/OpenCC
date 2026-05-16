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

#include "SerializableDict.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include "WinUtil.hpp"
#endif

namespace opencc {

FILE* OpenSerializableFileUtf8(const std::string& fileName,
                               const char* mode) {
#if defined(_WIN32) || defined(_WIN64)
  return _wfopen(internal::WideFromUtf8(fileName).c_str(),
                 internal::WideFromUtf8(mode).c_str());
#else
  return fopen(fileName.c_str(), mode);
#endif
}

} // namespace opencc
