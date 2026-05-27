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

#include <cstdio>
#include <list>
#include <mutex>
#include <sys/stat.h>
#include <unordered_map>

#if defined(_WIN32) || defined(_WIN64)
#include "WinUtil.hpp"
#endif

#include <rapidjson/document.h>

#include "Config.hpp"
#include "ConversionChain.hpp"
#include "Converter.hpp"
#include "DictGroup.hpp"
#include "Exception.hpp"
#include "MarisaDict.hpp"
#include "MaxMatchSegmentation.hpp"
#include "PluginSegmentation.hpp"
#include "TextDict.hpp"
#include "UTF8Util.hpp"

#ifdef ENABLE_DARTS
#include "DartsDict.hpp"
#endif

typedef rapidjson::GenericValue<rapidjson::UTF8<char>> JSONValue;

namespace opencc {

namespace {

std::string GetParentDirectory(const std::string& path);

std::mutex& DictCacheMutex() {
  static std::mutex mutex;
  return mutex;
}

std::unordered_map<std::string, std::weak_ptr<Dict>>& DictCache() {
  static std::unordered_map<std::string, std::weak_ptr<Dict>> cache;
  return cache;
}

void PruneExpiredDictCache() {
  std::unordered_map<std::string, std::weak_ptr<Dict>>& cache = DictCache();
  for (std::unordered_map<std::string, std::weak_ptr<Dict>>::iterator it =
           cache.begin();
       it != cache.end();) {
    if (it->second.expired()) {
      it = cache.erase(it);
    } else {
      ++it;
    }
  }
}

bool GetFileCacheKey(const std::string& path, std::string* cacheKey) {
#if defined(_WIN32) || defined(_WIN64)
  WIN32_FILE_ATTRIBUTE_DATA fileInfo;
  const std::wstring widePath = internal::WideFromUtf8(path);
  if (widePath.empty() ||
      !GetFileAttributesExW(widePath.c_str(), GetFileExInfoStandard,
                            &fileInfo)) {
    return false;
  }
#else
  struct stat statBuf;
  if (stat(path.c_str(), &statBuf) != 0) {
    return false;
  }
#endif
  *cacheKey = path;
  cacheKey->push_back('\n');
#if defined(_WIN32) || defined(_WIN64)
  cacheKey->append(
      std::to_string(static_cast<unsigned long long>(
          fileInfo.ftLastWriteTime.dwHighDateTime)));
  cacheKey->push_back('.');
  cacheKey->append(
      std::to_string(static_cast<unsigned long long>(
          fileInfo.ftLastWriteTime.dwLowDateTime)));
  cacheKey->push_back('\n');
  cacheKey->append(
      std::to_string(static_cast<unsigned long long>(fileInfo.nFileSizeHigh)));
  cacheKey->push_back('.');
  cacheKey->append(
      std::to_string(static_cast<unsigned long long>(fileInfo.nFileSizeLow)));
#else
  cacheKey->append(std::to_string(static_cast<long long>(statBuf.st_mtime)));
  cacheKey->push_back('.');
#if defined(__APPLE__) && defined(__MACH__)
  cacheKey->append(
      std::to_string(static_cast<long long>(statBuf.st_mtimespec.tv_nsec)));
#elif defined(st_mtime_nsec)
  cacheKey->append(
      std::to_string(static_cast<long long>(statBuf.st_mtime_nsec)));
#else
  cacheKey->append(
      std::to_string(static_cast<long long>(statBuf.st_mtim.tv_nsec)));
#endif
  cacheKey->push_back('\n');
  cacheKey->append(std::to_string(static_cast<long long>(statBuf.st_size)));
#endif
  return true;
}

FILE* OpenFileUtf8(const std::string& path, const char* mode) {
#if defined(_WIN32) || defined(_WIN64)
  return _wfopen(internal::WideFromUtf8(path).c_str(),
                 internal::WideFromUtf8(mode).c_str());
#else
  return fopen(path.c_str(), mode);
#endif
}

bool CanOpenFileUtf8(const std::string& path) {
  FILE* fp = OpenFileUtf8(path, "rb");
  if (fp == nullptr) {
    return false;
  }
  fclose(fp);
  return true;
}

std::string ReadFileUtf8(const std::string& path) {
  FILE* fp = OpenFileUtf8(path, "rb");
  if (fp == nullptr) {
    throw FileNotFound(path);
  }

  std::string content;
  char buffer[4096];
  for (;;) {
    const size_t read = fread(buffer, 1, sizeof(buffer), fp);
    if (read > 0) {
      content.append(buffer, read);
    }
    if (read < sizeof(buffer)) {
      if (ferror(fp)) {
        fclose(fp);
        throw FileNotFound(path);
      }
      break;
    }
  }
  fclose(fp);
  return content;
}

#if defined(_WIN32) || defined(_WIN64)
using internal::Utf8FromWide;
using internal::WideFromUtf8;

std::string NormalizeModulePath(const std::string& path) {
  if (path.empty()) {
    return "";
  }

  std::wstring widePath = WideFromUtf8(path);
  if (widePath.empty()) {
    return path;
  }

  HANDLE handle =
      CreateFileW(widePath.c_str(), 0,
                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                  nullptr, OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
  if (handle == INVALID_HANDLE_VALUE) {
    return path;
  }

  std::wstring finalPath(MAX_PATH, L'\0');
  for (;;) {
    DWORD copied =
        GetFinalPathNameByHandleW(handle, finalPath.data(),
                                  static_cast<DWORD>(finalPath.size()),
                                  FILE_NAME_NORMALIZED);
    if (copied == 0) {
      CloseHandle(handle);
      return path;
    }
    if (copied < finalPath.size()) {
      finalPath.resize(copied);
      break;
    }
    finalPath.resize(copied + 1);
  }
  CloseHandle(handle);

  const std::wstring uncPrefix = L"\\\\?\\UNC\\";
  const std::wstring localPrefix = L"\\\\?\\";
  if (finalPath.rfind(uncPrefix, 0) == 0) {
    finalPath = L"\\" + finalPath.substr(7);
  } else if (finalPath.rfind(localPrefix, 0) == 0) {
    finalPath = finalPath.substr(4);
  }
  return Utf8FromWide(finalPath);
}

std::string GetModulePath(HMODULE module) {
  std::wstring buffer(MAX_PATH, L'\0');
  for (;;) {
    DWORD copied =
        GetModuleFileNameW(module, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (copied == 0) {
      return "";
    }
    if (copied < buffer.size() - 1) {
      buffer.resize(copied);
      return NormalizeModulePath(Utf8FromWide(buffer));
    }
    buffer.resize(buffer.size() * 2);
  }
}

std::string GetCurrentProcessModulePath() {
  return GetModulePath(nullptr);
}

std::string GetCurrentLibraryModulePath() {
  HMODULE module = nullptr;
  if (!GetModuleHandleExW(
          GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
          reinterpret_cast<LPCWSTR>(&GetCurrentLibraryModulePath), &module)) {
    return "";
  }
  return GetModulePath(module);
}

void AppendWindowsPortableSearchPaths(std::vector<std::string>& paths,
                                      const std::string& modulePath) {
  const std::string parent = GetParentDirectory(modulePath);
  if (parent.empty()) {
    return;
  }
  paths.push_back(parent);
  paths.push_back(parent + "../share/opencc");
}
#endif

class ConfigInternal {
public:
  std::vector<std::string> paths;
  std::string configDirectory;

  const JSONValue& GetProperty(const JSONValue& doc, const char* name) {
    if (!doc.HasMember(name)) {
      throw InvalidFormat("Required property not found: " + std::string(name));
    }
    return doc[name];
  }

  const JSONValue& GetObjectProperty(const JSONValue& doc, const char* name) {
    const JSONValue& obj = GetProperty(doc, name);
    if (!obj.IsObject()) {
      throw InvalidFormat("Property must be an object: " + std::string(name));
    }
    return obj;
  }

  const JSONValue& GetArrayProperty(const JSONValue& doc, const char* name) {
    const JSONValue& obj = GetProperty(doc, name);
    if (!obj.IsArray()) {
      throw InvalidFormat("Property must be an array: " + std::string(name));
    }
    return obj;
  }

  const char* GetStringProperty(const JSONValue& doc, const char* name) {
    const JSONValue& obj = GetProperty(doc, name);
    if (!obj.IsString()) {
      throw InvalidFormat("Property must be a std::string: " +
                          std::string(name));
    }
    return obj.GetString();
  }

  template <typename DICT>
  DictPtr LoadDictWithPaths(const std::string& cachePrefix,
                            const std::string& fileName) {
    std::vector<std::string> candidates;
    candidates.push_back(fileName);
    for (const std::string& dirPath : paths) {
      candidates.push_back(dirPath + '/' + fileName);
    }

    for (const std::string& path : candidates) {
      std::string cacheKey = cachePrefix;
      cacheKey.push_back('\n');
      if (!GetFileCacheKey(path, &cacheKey)) {
        continue;
      }
      {
        std::lock_guard<std::mutex> lock(DictCacheMutex());
        PruneExpiredDictCache();
        const auto cached = DictCache().find(cacheKey);
        if (cached != DictCache().end()) {
          DictPtr dict = cached->second.lock();
          if (dict != nullptr) {
            return dict;
          }
        }
      }

      std::shared_ptr<DICT> dict;
      if (SerializableDict::TryLoadFromFile<DICT>(path, &dict)) {
        std::lock_guard<std::mutex> lock(DictCacheMutex());
        PruneExpiredDictCache();
        std::weak_ptr<Dict>& cached = DictCache()[cacheKey];
        DictPtr cachedDict = cached.lock();
        if (cachedDict == nullptr) {
          cached = dict;
          return dict;
        }
        return cachedDict;
      }
    }
    throw FileNotFound(fileName);
  }

  DictPtr LoadDictFromFile(const std::string& type,
                           const std::string& fileName) {
    if (type == "text") {
      DictPtr dict = LoadDictWithPaths<TextDict>("text", fileName);
      return MarisaDict::NewFromDict(*dict.get());
    }
#ifdef ENABLE_DARTS
    if (type == "ocd") {
      return LoadDictWithPaths<DartsDict>("ocd", fileName);
    }
#endif
    if (type == "ocd2") {
      return LoadDictWithPaths<MarisaDict>("ocd2", fileName);
    }
    throw InvalidFormat("Unknown dictionary type: " + type);
    return nullptr;
  }

  DictPtr ParseDict(const JSONValue& doc) {
    // Required: type
    std::string type = GetStringProperty(doc, "type");

    if (type == "group") {
      std::list<DictPtr> dicts;
      const JSONValue& docs = GetArrayProperty(doc, "dicts");
      for (rapidjson::SizeType i = 0; i < docs.Size(); i++) {
        if (docs[i].IsObject()) {
          DictPtr dict = ParseDict(docs[i]);
          dicts.push_back(dict);
        } else {
          throw InvalidFormat("Element of the array must be an object");
        }
      }
      return DictGroupPtr(new DictGroup(dicts));
    } else {
      std::string fileName = GetStringProperty(doc, "file");
      DictPtr dict = LoadDictFromFile(type, fileName);
      return dict;
    }
  }

  SegmentationPtr ParseSegmentation(const JSONValue& doc) {
    SegmentationPtr segmentation;

    // Required: type
    std::string type = GetStringProperty(doc, "type");
    if (type == "mmseg") {
      // Required: dict
      DictPtr dict = ParseDict(GetObjectProperty(doc, "dict"));
      segmentation = SegmentationPtr(new MaxMatchSegmentation(dict));
    } else {
      PluginConfigPairs configPairs;
      configPairs.push_back(std::make_pair("__config_dir", configDirectory));
      if (doc.HasMember("resources")) {
        const JSONValue& resources = GetObjectProperty(doc, "resources");
        for (auto it = resources.MemberBegin(); it != resources.MemberEnd();
             ++it) {
          if (!it->value.IsString()) {
            throw InvalidFormat("Segmentation resource must be a string: " +
                                std::string(it->name.GetString()));
          }
          configPairs.push_back(std::make_pair(it->name.GetString(),
                                               it->value.GetString()));
        }
      }
      for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
        const std::string key = it->name.GetString();
        if (key == "type" || key == "resources") {
          continue;
        }
        if (!it->value.IsString()) {
          throw InvalidFormat("Segmentation plugin property must be a string: " +
                              key);
        }
        configPairs.push_back(std::make_pair(key, it->value.GetString()));
      }
      segmentation = CreatePluginSegmentation(type, configPairs);
    }
    return segmentation;
  }

  ConversionPtr ParseConversion(const JSONValue& doc) {
    // Required: dict
    DictPtr dict = ParseDict(GetObjectProperty(doc, "dict"));
    ConversionPtr conversion(new Conversion(dict));

    return conversion;
  }

  ConversionChainPtr ParseConversionChain(const JSONValue& docs) {
    std::list<ConversionPtr> conversions;
    for (rapidjson::SizeType i = 0; i < docs.Size(); i++) {
      const JSONValue& doc = docs[i];
      if (doc.IsObject()) {
        ConversionPtr conversion = ParseConversion(doc);
        conversions.push_back(conversion);
      } else {
      }
    }
    ConversionChainPtr chain(new ConversionChain(conversions));
    return chain;
  }

  std::string FindConfigFile(std::string fileName) {
    // Working directory
    if (CanOpenFileUtf8(fileName)) {
      return fileName;
    }
    // Package data directory
    if (PACKAGE_DATA_DIRECTORY != "") {
      std::string prefixedFileName = PACKAGE_DATA_DIRECTORY + fileName;
      if (CanOpenFileUtf8(prefixedFileName)) {
        return prefixedFileName;
      }
      prefixedFileName += ".json";
      if (CanOpenFileUtf8(prefixedFileName)) {
        return prefixedFileName;
      }
    }

    for (const std::string& dirPath : paths) {
      std::string path = dirPath + '/' + fileName;
      if (CanOpenFileUtf8(path)) {
        return path;
      }
    }

    const char* envPath = std::getenv("OPENCC_DATA_DIR");
    if (envPath != nullptr) {
      auto path = std::string(envPath) + '/' + fileName;
      if (CanOpenFileUtf8(path)) {
        return path;
      }
    }

    throw FileNotFound(fileName);
  }
};

std::string GetParentDirectory(const std::string& path) {
  size_t pos = path.rfind('/', path.length() - 1);
  if (pos == std::string::npos) {
    pos = path.rfind('\\', path.length() - 1);
  }
  if (pos == std::string::npos) {
    return "";
  }
  return path.substr(0, pos + 1);
}

bool isRegularFile(const std::string& path) {
#if defined(_WIN32) || defined(_WIN64)
  const DWORD attributes = GetFileAttributesW(WideFromUtf8(path).c_str());
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

} // namespace

Config::Config() : internal(new ConfigInternal()) {}

Config::~Config() { delete reinterpret_cast<ConfigInternal*>(internal); }

ConverterPtr Config::NewFromFile(const std::string& fileName) {
  return NewFromFile(fileName, std::vector<std::string>{}, nullptr);
}

ConverterPtr Config::NewFromFile(const std::string& fileName,
                                 const std::vector<std::string>& paths,
                                 const char* argv0) {
  ConfigInternal* impl = reinterpret_cast<ConfigInternal*>(internal);
  impl->paths = paths;
  if (argv0 != nullptr) {
    std::string parent = GetParentDirectory(argv0);
    if (!parent.empty()) {
      impl->paths.push_back(parent);
    }
  }
#if defined(_WIN32) || defined(_WIN64)
  if (argv0 != nullptr) {
    AppendWindowsPortableSearchPaths(impl->paths, argv0);
  }
  AppendWindowsPortableSearchPaths(impl->paths, GetCurrentProcessModulePath());
  AppendWindowsPortableSearchPaths(impl->paths, GetCurrentLibraryModulePath());
#endif
  if (PACKAGE_DATA_DIRECTORY != "") {
    impl->paths.push_back(PACKAGE_DATA_DIRECTORY);
  }
  std::string prefixedFileName = impl->FindConfigFile(fileName);
  if (!isRegularFile(prefixedFileName)) {
    throw FileNotFound(prefixedFileName);
  }
  std::string content = ReadFileUtf8(prefixedFileName);

#if defined(_WIN32) || defined(_WIN64)
  UTF8Util::ReplaceAll(prefixedFileName, "\\", "/");
#endif // if defined(_WIN32) || defined(_WIN64)
  size_t slashPos = prefixedFileName.rfind("/");
  std::string configDirectory = "";
  if (slashPos != std::string::npos) {
    configDirectory = prefixedFileName.substr(0, slashPos) + "/";
  }
  if (!configDirectory.empty()) {
    impl->paths.push_back(configDirectory);
  }
  impl->configDirectory = configDirectory;
  return NewFromString(content, impl->paths);
}

ConverterPtr Config::NewFromString(const std::string& json,
                                   const std::string& configDirectory) {
  std::vector<std::string> paths;
  if (!configDirectory.empty()) {
    if (configDirectory.back() == '/' || configDirectory.back() == '\\') {
      paths.push_back(configDirectory);
    } else {
      paths.push_back(configDirectory + '/');
    }
  }
  return NewFromString(json, paths);
}

ConverterPtr Config::NewFromString(const std::string& json,
                                   const std::vector<std::string>& paths) {
  rapidjson::Document doc;

  doc.ParseInsitu<0>(const_cast<char*>(json.c_str()));
  if (doc.HasParseError()) {
    throw InvalidFormat("Error parsing JSON"); // doc.GetErrorOffset()
  }
  if (!doc.IsObject()) {
    throw InvalidFormat("Root of configuration must be an object");
  }

  // Optional: name
  std::string name;
  if (doc.HasMember("name") && doc["name"].IsString()) {
    name = doc["name"].GetString();
  }

  ConfigInternal* impl = reinterpret_cast<ConfigInternal*>(internal);
  impl->paths = paths;
  if (impl->configDirectory.empty()) {
    impl->configDirectory = paths.empty() ? "" : paths.front();
  }

  // Required: segmentation
  SegmentationPtr segmentation =
      impl->ParseSegmentation(impl->GetObjectProperty(doc, "segmentation"));

  // Required: conversion_chain
  ConversionChainPtr chain = impl->ParseConversionChain(
      impl->GetArrayProperty(doc, "conversion_chain"));
  return ConverterPtr(new Converter(name, segmentation, chain));
}

}; // namespace opencc
