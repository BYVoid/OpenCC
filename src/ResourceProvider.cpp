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

#include <cstdint>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include "WinUtil.hpp"
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#include "Exception.hpp"

namespace opencc {
namespace {

bool IsSeparator(char ch) { return ch == '/' || ch == '\\'; }

uint16_t ReadLe16(const unsigned char* data) {
  return static_cast<uint16_t>(data[0]) |
         (static_cast<uint16_t>(data[1]) << 8);
}

uint32_t ReadLe32(const unsigned char* data) {
  return static_cast<uint32_t>(data[0]) |
         (static_cast<uint32_t>(data[1]) << 8) |
         (static_cast<uint32_t>(data[2]) << 16) |
         (static_cast<uint32_t>(data[3]) << 24);
}

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

std::string JoinPath(const std::string& root, const std::string& resource) {
  if (root.empty()) {
    return resource;
  }
  if (IsSeparator(root.back())) {
    return root + resource;
  }
  return root + "/" + resource;
}

std::string NormalizeResourceName(std::string_view resourceName) {
  std::string normalized(resourceName);
  for (char& ch : normalized) {
    if (ch == '\\') {
      ch = '/';
    }
  }
  while (!normalized.empty() && normalized.front() == '/') {
    normalized.erase(normalized.begin());
  }
  return normalized;
}

bool IsSafeZipResourceName(const std::string& name) {
  if (name.empty() || IsAbsolutePath(name)) {
    return false;
  }
  size_t start = 0;
  for (;;) {
    const size_t pos = name.find('/', start);
    const std::string part = name.substr(start, pos - start);
    if (part.empty() || part == "." || part == "..") {
      return false;
    }
    if (pos == std::string::npos) {
      return true;
    }
    start = pos + 1;
  }
}

std::string BaseName(const std::string& path) {
  const size_t pos = path.find_last_of('/');
  if (pos == std::string::npos) {
    return path;
  }
  return path.substr(pos + 1);
}

#if defined(_WIN32) || defined(_WIN64)
std::vector<unsigned char> ReadBinaryFile(const std::string& path) {
  FILE* file = _wfopen(internal::WideFromUtf8(path).c_str(), L"rb");
  if (file == nullptr) {
    throw FileNotFound(path);
  }
  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    throw FileNotFound(path);
  }
  const long fileSize = ftell(file);
  if (fileSize < 0) {
    fclose(file);
    throw FileNotFound(path);
  }
  if (fseek(file, 0, SEEK_SET) != 0) {
    fclose(file);
    throw FileNotFound(path);
  }
  std::vector<unsigned char> data(static_cast<size_t>(fileSize));
  if (!data.empty() &&
      fread(data.data(), 1, data.size(), file) != data.size()) {
    fclose(file);
    throw FileNotFound(path);
  }
  fclose(file);
  return data;
}
#endif

struct ZipEntry {
  uint16_t method;
  uint32_t compressedSize;
  uint32_t uncompressedSize;
  uint32_t dataOffset;
};

class MappedZipArchive {
public:
  explicit MappedZipArchive(const std::string& path) : fileName(path) {
#if defined(_WIN32) || defined(_WIN64)
    buffer = ReadBinaryFile(path);
    data = buffer.empty() ? nullptr : buffer.data();
    size = buffer.size();
#else
    fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
      throw FileNotFound(path);
    }
    struct stat info;
    if (fstat(fd, &info) != 0 || info.st_size < 0) {
      close(fd);
      fd = -1;
      throw FileNotFound(path);
    }
    size = static_cast<size_t>(info.st_size);
    if (size == 0) {
      close(fd);
      fd = -1;
      data = nullptr;
      return;
    }
    void* mapped = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
      close(fd);
      fd = -1;
      throw FileNotFound(path);
    }
    data = static_cast<const unsigned char*>(mapped);
#endif
  }

  MappedZipArchive(const MappedZipArchive&) = delete;
  MappedZipArchive& operator=(const MappedZipArchive&) = delete;

  ~MappedZipArchive() {
#if !defined(_WIN32) && !defined(_WIN64)
    if (data != nullptr) {
      munmap(const_cast<unsigned char*>(data), size);
    }
    if (fd >= 0) {
      close(fd);
    }
#endif
  }

  const std::string fileName;
  const unsigned char* data = nullptr;
  size_t size = 0;

private:
#if defined(_WIN32) || defined(_WIN64)
  std::vector<unsigned char> buffer;
#else
  int fd = -1;
#endif
};

} // namespace

struct ZipResourceProvider::Internal {
  explicit Internal(const std::string& zipFileName_)
      : archive(std::make_shared<MappedZipArchive>(zipFileName_)) {
    Index();
  }

  void Index();

  std::shared_ptr<MappedZipArchive> archive;
  std::unordered_map<std::string, ZipEntry> entries;
};

void ZipResourceProvider::Internal::Index() {
  const unsigned char* data = archive->data;
  const size_t size = archive->size;
  if (size < 22) {
    throw InvalidFormat("Invalid zip archive: " + archive->fileName);
  }

  const size_t maxCommentSize = 65535;
  const size_t searchStart =
      size > maxCommentSize + 22 ? size - maxCommentSize - 22 : 0;
  size_t eocdOffset = std::string::npos;
  for (size_t pos = size - 22;; pos--) {
    if (data[pos] == 0x50 && data[pos + 1] == 0x4b && data[pos + 2] == 0x05 &&
        data[pos + 3] == 0x06) {
      eocdOffset = pos;
      break;
    }
    if (pos == searchStart) {
      break;
    }
  }
  if (eocdOffset == std::string::npos) {
    throw InvalidFormat("Invalid zip archive: " + archive->fileName);
  }

  const uint16_t entryCount = ReadLe16(&data[eocdOffset + 10]);
  const uint32_t centralDirectorySize = ReadLe32(&data[eocdOffset + 12]);
  const uint32_t centralDirectoryOffset = ReadLe32(&data[eocdOffset + 16]);
  if (centralDirectoryOffset > size ||
      centralDirectorySize > size - centralDirectoryOffset) {
    throw InvalidFormat("Invalid zip central directory: " + archive->fileName);
  }

  size_t pos = centralDirectoryOffset;
  for (uint16_t i = 0; i < entryCount; i++) {
    if (pos + 46 > size || ReadLe32(&data[pos]) != 0x02014b50) {
      throw InvalidFormat("Invalid zip central directory: " + archive->fileName);
    }
    const uint16_t method = ReadLe16(&data[pos + 10]);
    const uint32_t compressedSize = ReadLe32(&data[pos + 20]);
    const uint32_t uncompressedSize = ReadLe32(&data[pos + 24]);
    const uint16_t fileNameLength = ReadLe16(&data[pos + 28]);
    const uint16_t extraLength = ReadLe16(&data[pos + 30]);
    const uint16_t commentLength = ReadLe16(&data[pos + 32]);
    const uint32_t localHeaderOffset = ReadLe32(&data[pos + 42]);
    const size_t next = pos + 46 + fileNameLength + extraLength + commentLength;
    if (next > size) {
      throw InvalidFormat("Invalid zip central directory: " + archive->fileName);
    }
    const std::string name(
        reinterpret_cast<const char*>(&data[pos + 46]), fileNameLength);
    const std::string normalized = NormalizeResourceName(name);
    if (!normalized.empty() && normalized.back() != '/' &&
        IsSafeZipResourceName(normalized)) {
      if (localHeaderOffset + 30 > size ||
          ReadLe32(&data[localHeaderOffset]) != 0x04034b50) {
        throw InvalidFormat("Invalid zip local header: " + archive->fileName);
      }
      const uint16_t localNameLength = ReadLe16(&data[localHeaderOffset + 26]);
      const uint16_t localExtraLength = ReadLe16(&data[localHeaderOffset + 28]);
      const size_t dataOffset =
          localHeaderOffset + 30 + localNameLength + localExtraLength;
      if (dataOffset > size || compressedSize > size - dataOffset) {
        throw InvalidFormat("Invalid zip entry data: " + archive->fileName);
      }
      entries[normalized] = ZipEntry{
          method, compressedSize, uncompressedSize,
          static_cast<uint32_t>(dataOffset)};
    }
    pos = next;
  }
}

ResourceProvider::Resource::Resource(std::string name_, const char* data_,
                                     size_t size_,
                                     std::shared_ptr<const void> owner_,
                                     std::string cacheKey_)
    : name(std::move(name_)), data(data_), size(size_),
      owner(std::move(owner_)), cacheKey(std::move(cacheKey_)) {}

std::shared_ptr<const ResourceProvider::Resource>
ResourceProvider::GetResource(std::string_view resourceName) const {
  const std::string path = Resolve(resourceName);
  std::shared_ptr<std::string> content(new std::string);
  std::ifstream input(path, std::ios::binary);
  if (!input) {
    throw FileNotFound(path);
  }
  content->assign(std::istreambuf_iterator<char>(input),
                  std::istreambuf_iterator<char>());
  if (input.bad()) {
    throw FileNotFound(path);
  }

  std::string cacheKey = path;
  cacheKey.push_back('\n');
  cacheKey.append(std::to_string(content->size()));
  return std::make_shared<Resource>(path, content->data(), content->size(),
                                    content, cacheKey);
}

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

ZipResourceProvider::ZipResourceProvider(std::string zipFileName)
    : internal(new Internal(zipFileName)) {}

ZipResourceProvider::~ZipResourceProvider() = default;

std::string ZipResourceProvider::Resolve(std::string_view resourceName) const {
  throw FileNotFound(std::string(resourceName));
}

std::shared_ptr<const ResourceProvider::Resource>
ZipResourceProvider::GetResource(std::string_view resourceName) const {
  const std::string normalized = NormalizeResourceName(resourceName);
  if (!IsSafeZipResourceName(normalized)) {
    throw FileNotFound(std::string(resourceName));
  }

  auto entry = internal->entries.find(normalized);
  if (entry == internal->entries.end()) {
    const std::string baseName = BaseName(normalized);
    if (baseName != normalized) {
      entry = internal->entries.find(baseName);
    }
  }
  if (entry == internal->entries.end()) {
    throw FileNotFound(normalized);
  }
  if (entry->second.method != 0) {
    throw InvalidFormat("Unsupported zip compression method for " + entry->first);
  }
  if (entry->second.compressedSize != entry->second.uncompressedSize) {
    throw InvalidFormat("Invalid stored zip entry size for " + entry->first);
  }

  const char* data = reinterpret_cast<const char*>(
      internal->archive->data + entry->second.dataOffset);
  std::string cacheKey = internal->archive->fileName;
  cacheKey.push_back('\n');
  cacheKey.append(entry->first);
  cacheKey.push_back('\n');
  cacheKey.append(std::to_string(entry->second.uncompressedSize));
  return std::make_shared<ResourceProvider::Resource>(
      entry->first, data, entry->second.uncompressedSize, internal->archive,
      cacheKey);
}

} // namespace opencc
