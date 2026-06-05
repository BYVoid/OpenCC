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

#include "ResourceProvider.hpp"

#include <sstream>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)
#include "WinUtil.hpp"
#endif

#include "Exception.hpp"

namespace opencc {
namespace {

bool IsSeparator(char ch) { return ch == '/' || ch == '\\'; }

bool IsAbsolutePath(const std::string& path) {
  if (path.empty()) {
    return false;
  }
  if (IsSeparator(path[0])) {
    return true;
  }
#if defined(_WIN32) || defined(_WIN64)
  return path.size() >= 3 && path[1] == ':' && IsSeparator(path[2]);
#else
  return false;
#endif
}

bool IsRegularFileUtf8(const std::string& path) {
#if defined(_WIN32) || defined(_WIN64)
  const DWORD attributes =
      GetFileAttributesW(internal::WideFromUtf8(path).c_str());
  return attributes != INVALID_FILE_ATTRIBUTES &&
         (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
  struct stat info;
  if (stat(path.c_str(), &info) != 0) {
    return false;
  }
  return (info.st_mode & S_IFMT) == S_IFREG;
#endif
}

bool HasParentReference(const std::string& path) {
  size_t start = 0;
  while (start <= path.size()) {
    size_t end = start;
    while (end < path.size() && !IsSeparator(path[end])) {
      ++end;
    }
    if (path.substr(start, end - start) == "..") {
      return true;
    }
    start = end + 1;
  }
  return false;
}

std::string JoinPath(const std::string& root, const std::string& resource) {
  if (root.empty()) {
    return resource;
  }
  if (IsSeparator(root.back())) {
    return root + resource;
  }
  return root + "/" + resource;
}

} // namespace

FilesystemResourceProvider::FilesystemResourceProvider(
    std::vector<std::string> searchPaths_)
    : searchPaths(std::move(searchPaths_)) {}

std::string
FilesystemResourceProvider::Resolve(std::string_view resourceName) const {
  const std::string resourcePath(resourceName);
  if (resourcePath.empty()) {
    throw FileNotFound(resourcePath);
  }

  if (IsAbsolutePath(resourcePath)) {
    if (IsRegularFileUtf8(resourcePath)) {
      return resourcePath;
    }
    throw FileNotFound(resourcePath);
  }

  if (HasParentReference(resourcePath)) {
    throw FileNotFound(resourcePath);
  }

  std::ostringstream searched;
  for (size_t i = 0; i < searchPaths.size(); i++) {
    const std::string& root = searchPaths[i];
    if (i != 0) {
      searched << ", ";
    }
    searched << root;

    const std::string candidate = JoinPath(root, resourcePath);
    if (IsRegularFileUtf8(candidate)) {
      return candidate;
    }
  }

  throw FileNotFound(resourcePath + " (searched: " + searched.str() + ")");
}

} // namespace opencc
