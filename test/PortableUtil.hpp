/*
 * Open Chinese Convert
 *
 * Copyright 2015-2026 Carbo Kuo and contributors
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

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

namespace opencc {

inline char* portable_getcwd() {
#ifdef _WIN32
  return _getcwd(nullptr, 0);
#else
  return getcwd(nullptr, 0);
#endif
}

inline int portable_chdir(const char* path) {
#ifdef _WIN32
  return _chdir(path);
#else
  return chdir(path);
#endif
}

inline void portable_putenv(const char* key, const char* value) {
#ifdef _WIN32
  _putenv_s(key, value);
#else
  setenv(key, value, 1);
#endif
}

inline void portable_unsetenv(const char* key) {
#ifdef _WIN32
  _putenv_s(key, "");
#else
  unsetenv(key);
#endif
}

} // namespace opencc
