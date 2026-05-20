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

#include <cstdio>
#include <string>
#include <vector>

namespace opencc {
namespace tools {

FILE* OpenFileUtf8(const std::string& fileName, const char* mode);
FILE* CreateTempFileNearUtf8(const std::string& targetFileName,
                             std::string* fileName);
bool GetRealPathUtf8(const std::string& fileName, std::string* realPath);
bool HasMultipleLinksUtf8(const std::string& fileName);
bool IsSameFileUtf8(const std::string& first, const std::string& second);
int PreserveMetadataUtf8(const std::string& source,
                         const std::string& target);
int ReplaceFileUtf8(const std::string& source, const std::string& target);
int RemoveFileUtf8(const std::string& fileName);

#ifdef _WIN32
std::vector<std::string> GetUtf8CommandLineArgs();
#endif

} // namespace tools
} // namespace opencc
