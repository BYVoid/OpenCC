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

#include "PluginSegmentation.hpp"

#include "Exception.hpp"
#include "Segmentation.hpp"
#include "Segments.hpp"
#include "UTF8Util.hpp"
#include "plugin/OpenCCPlugin.h"

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <unordered_map>

#if defined(_WIN32) || defined(_WIN64)
#include "WinUtil.hpp"
#else
#include <dlfcn.h>
#endif

namespace opencc {

namespace {

std::string DirName(const std::string& path) {
  const std::string::size_type pos = path.find_last_of("/\\");
  if (pos == std::string::npos) {
    return "";
  }
  return path.substr(0, pos);
}

std::string JoinPath(const std::string& left, const std::string& right) {
  if (left.empty()) {
    return right;
  }
  const char last = left.back();
  if (last == '/' || last == '\\') {
    return left + right;
  }
  return left + "/" + right;
}

void AppendUnique(std::vector<std::string>& values, const std::string& value) {
  if (value.empty()) {
    return;
  }
  for (const std::string& existing : values) {
    if (existing == value) {
      return;
    }
  }
  values.push_back(value);
}

bool IsTruthy(const char* value) {
  if (value == nullptr) {
    return false;
  }
  const std::string text(value);
  return text == "1" || text == "true" || text == "TRUE" || text == "yes" ||
         text == "on";
}

bool IsReadableFile(const std::string& path) {
#if defined(_WIN32) || defined(_WIN64)
  const DWORD attributes =
      GetFileAttributesW(internal::WideFromUtf8(path).c_str());
  return attributes != INVALID_FILE_ATTRIBUTES &&
         (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
  std::ifstream ifs(path.c_str(), std::ios::binary);
  return ifs.is_open();
#endif
}

std::vector<std::string> SplitSearchPath(const char* raw) {
  std::vector<std::string> entries;
  if (raw == nullptr || *raw == '\0') {
    return entries;
  }
  const char separator =
#if defined(_WIN32) || defined(_WIN64)
      ';';
#else
      ':';
#endif
  std::string path(raw);
  std::string::size_type start = 0;
  while (start <= path.size()) {
    const std::string::size_type end = path.find(separator, start);
    const std::string entry = path.substr(start, end - start);
    if (!entry.empty()) {
      entries.push_back(entry);
    }
    if (end == std::string::npos) {
      break;
    }
    start = end + 1;
  }
  return entries;
}

#if defined(_WIN32) || defined(_WIN64)
using internal::Utf8FromWide;
using internal::WideFromUtf8;

std::string GetCurrentLibraryDirectory() {
  HMODULE module = nullptr;
  if (!GetModuleHandleExW(
          GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
          reinterpret_cast<LPCWSTR>(&GetCurrentLibraryDirectory), &module)) {
    return "";
  }
  std::wstring buffer(MAX_PATH, L'\0');
  for (;;) {
    const DWORD copied = GetModuleFileNameW(module, buffer.data(),
                                            static_cast<DWORD>(buffer.size()));
    if (copied == 0) {
      return "";
    }
    if (copied < buffer.size() - 1) {
      buffer.resize(copied);
      return DirName(Utf8FromWide(buffer));
    }
    buffer.resize(buffer.size() * 2);
  }
}
#else
std::string GetCurrentLibraryDirectory() {
  Dl_info info;
  if (dladdr(reinterpret_cast<void*>(&GetCurrentLibraryDirectory), &info) == 0 ||
      info.dli_fname == nullptr) {
    return "";
  }
  return DirName(info.dli_fname);
}
#endif

class SharedLibrary {
public:
  explicit SharedLibrary(const std::string& path) {
#if defined(_WIN32) || defined(_WIN64)
    handle_ = LoadLibraryExW(
        WideFromUtf8(path).c_str(), nullptr,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (handle_ == nullptr) {
      throw Exception("Failed to load plugin library: " + path);
    }
#else
    handle_ = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (handle_ == nullptr) {
      throw Exception("Failed to load plugin library: " + path);
    }
#endif
  }

  ~SharedLibrary() {
#if defined(_WIN32) || defined(_WIN64)
    if (handle_ != nullptr) {
      FreeLibrary(handle_);
    }
#else
    if (handle_ != nullptr) {
      dlclose(handle_);
    }
#endif
  }

  void* FindSymbol(const char* symbolName) const {
#if defined(_WIN32) || defined(_WIN64)
    FARPROC symbol = GetProcAddress(handle_, symbolName);
    if (symbol == nullptr) {
      throw Exception("Plugin symbol not found: " + std::string(symbolName));
    }
    return reinterpret_cast<void*>(symbol);
#else
    dlerror();
    void* symbol = dlsym(handle_, symbolName);
    if (symbol == nullptr) {
      throw Exception("Plugin symbol not found: " + std::string(symbolName));
    }
    return symbol;
#endif
  }

private:
#if defined(_WIN32) || defined(_WIN64)
  HMODULE handle_ = nullptr;
#else
  void* handle_ = nullptr;
#endif
};

struct PluginLibrary {
  std::shared_ptr<SharedLibrary> library;
  const opencc_segmentation_plugin_v2* descriptor = nullptr;
};

size_t RequiredDescriptorSize() {
  return offsetof(opencc_segmentation_plugin_v2, free_error) +
         sizeof(((opencc_segmentation_plugin_v2*)nullptr)->free_error);
}

std::string TakeErrorMessage(const opencc_segmentation_plugin_v2* descriptor,
                             opencc_error_t* error,
                             const std::string& fallback) {
  if (error == nullptr) {
    return fallback;
  }
  const std::string message =
      error->message == nullptr ? fallback : std::string(error->message);
  if (descriptor->free_error != nullptr) {
    descriptor->free_error(error);
  }
  return message;
}

[[noreturn]] void ThrowPluginError(const opencc_segmentation_plugin_v2* descriptor,
                                   opencc_error_t* error,
                                   const std::string& pluginType,
                                   const std::string& fallbackPrefix,
                                   const std::string& fallbackMessage) {
  const std::string message =
      TakeErrorMessage(descriptor, error, fallbackMessage);
  throw Exception(fallbackPrefix + " '" + pluginType + "': " + message);
}

SegmentsPtr BuildSegmentsFromLengths(const std::string& text,
                                     const opencc_segment_length_array_t& lengths) {
  SegmentsPtr segments(new Segments);
  const char* bytes = text.c_str();
  const size_t inputSize = std::strlen(bytes);
  size_t byteOffset = 0;

  for (size_t i = 0; i < lengths.segment_count; i++) {
    const uint32_t codepointLength = lengths.codepoint_lengths[i];
    if (codepointLength == 0) {
      throw Exception("Segmentation plugin returned a zero-length segment.");
    }

    const size_t segmentStart = byteOffset;
    for (uint32_t j = 0; j < codepointLength; j++) {
      if (byteOffset >= inputSize) {
        throw Exception("Segmentation plugin returned lengths past end of input.");
      }
      const size_t charLength = UTF8Util::NextCharLengthNoException(bytes + byteOffset);
      if (charLength == 0 || byteOffset + charLength > inputSize) {
        throw Exception("Input text is not valid UTF-8.");
      }
      byteOffset += charLength;
    }

    segments->AddSegment(text.substr(segmentStart, byteOffset - segmentStart));
  }

  if (byteOffset != inputSize) {
    throw Exception("Segmentation plugin lengths do not cover the full input. "
                    "Consumed bytes: " + std::to_string(byteOffset) +
                    ", total bytes: " + std::to_string(inputSize) +
                    ", segments: " + std::to_string(lengths.segment_count) + ".");
  }
  return segments;
}

class PluginSegmentationAdapter : public Segmentation {
public:
  PluginSegmentationAdapter(std::shared_ptr<PluginLibrary> plugin,
                            opencc_segmentation_handle_t* handle)
      : plugin_(std::move(plugin)), handle_(handle) {}

  ~PluginSegmentationAdapter() override {
    if (handle_ != nullptr) {
      plugin_->descriptor->destroy(handle_);
      handle_ = nullptr;
    }
  }

  SegmentsPtr Segment(const std::string& text) const override {
    opencc_segment_length_array_t segmentLengths = {};
    segmentLengths.struct_size = sizeof(segmentLengths);
    opencc_error_t error = {};
    error.struct_size = sizeof(error);
    opencc_segmentation_segment_args_t args = {};
    args.struct_size = sizeof(args);
    args.handle = handle_;
    args.utf8_text = text.c_str();
    args.segment_lengths = &segmentLengths;
    args.error = &error;
    const int status = plugin_->descriptor->segment(&args);

    struct SegmentLengthGuard {
      const opencc_segmentation_plugin_v2* desc;
      opencc_segment_length_array_t* arr;
      ~SegmentLengthGuard() { desc->free_segment_lengths(arr); }
    } guard{plugin_->descriptor, &segmentLengths};

    if (status != 0) {
      ThrowPluginError(plugin_->descriptor, &error,
                       plugin_->descriptor->segmentation_type,
                       "Segmentation plugin failed", "unknown error");
    }
    return BuildSegmentsFromLengths(text, segmentLengths);
  }

private:
  std::shared_ptr<PluginLibrary> plugin_;
  mutable opencc_segmentation_handle_t* handle_ = nullptr;
};

class PluginManager {
public:
  std::shared_ptr<PluginLibrary> LoadByType(const std::string& type) {
    if (IsTruthy(std::getenv("OPENCC_DISABLE_PLUGINS"))) {
      throw Exception("Segmentation plugin loading is disabled.");
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cache_.find(type);
    if (it != cache_.end()) {
      return it->second;
    }

    const std::vector<std::string> searchDirs = BuildSearchDirs();
    const std::vector<std::string> fileNames = GetPluginFileNames(type);
    std::ostringstream attempted;
    bool first = true;
    for (const auto& dir : searchDirs) {
      std::vector<std::string> matches;
      for (const auto& fileName : fileNames) {
        const std::string candidate = JoinPath(dir, fileName);
        if (!first) {
          attempted << ", ";
        }
        first = false;
        attempted << candidate;
        if (IsReadableFile(candidate)) {
          matches.push_back(candidate);
        }
      }

      if (matches.size() > 1) {
        throw Exception("Multiple segmentation plugin libraries found for '" +
                        type + "' in directory '" + dir +
                        "'. Keep only one matching DLL per plugin type.");
      }
      if (matches.empty()) {
        continue;
      }
      try {
        std::shared_ptr<PluginLibrary> plugin =
            LoadFromPath(matches.front(), type);
        cache_[type] = plugin;
        return plugin;
      } catch (const Exception&) {
      }
    }
    throw Exception("Segmentation plugin '" + type +
                    "' not found. Searched: " + attempted.str());
  }

  std::shared_ptr<PluginLibrary> LoadByPath(const std::string& path,
                                            const std::string& type) {
    if (IsTruthy(std::getenv("OPENCC_DISABLE_PLUGINS"))) {
      throw Exception("Segmentation plugin loading is disabled.");
    }

    std::lock_guard<std::mutex> lock(mutex_);
    // Use the explicit path as cache key (not type) to avoid collisions
    // when the same plugin type is loaded from different library paths.
    auto it = cache_.find(path);
    if (it != cache_.end()) {
      return it->second;
    }
    std::shared_ptr<PluginLibrary> plugin = LoadFromPath(path, type);
    cache_[path] = plugin;
    return plugin;
  }

private:
  static std::vector<std::string> GetPluginFileNames(const std::string& type) {
#if defined(_WIN32) || defined(_WIN64)
    return {
        "opencc-" + type + ".dll",
        "libopencc-" + type + ".dll",
        "msys-opencc-" + type + ".dll",
    };
#elif defined(__APPLE__)
    return {"libopencc-" + type + ".dylib"};
#else
    return {"libopencc-" + type + ".so"};
#endif
  }

  static std::vector<std::string> BuildSearchDirs() {
    std::vector<std::string> dirs =
        SplitSearchPath(std::getenv("OPENCC_SEGMENTATION_PLUGIN_PATH"));
    AppendUnique(dirs, PACKAGE_SEGMENTATION_PLUGIN_DIRECTORY);

    const std::string currentLibraryDir = GetCurrentLibraryDirectory();
    if (!currentLibraryDir.empty()) {
#if defined(_WIN32) || defined(_WIN64)
      AppendUnique(dirs, JoinPath(currentLibraryDir, "plugins"));
#else
      AppendUnique(dirs, JoinPath(currentLibraryDir, "opencc/plugins"));
#endif
      AppendUnique(dirs, currentLibraryDir);
    }
    return dirs;
  }

  static std::shared_ptr<PluginLibrary> LoadFromPath(const std::string& path,
                                                     const std::string& type) {
    std::shared_ptr<SharedLibrary> library(new SharedLibrary(path));
    typedef const opencc_segmentation_plugin_v2* (*EntryPoint)(void);
    EntryPoint entryPoint = reinterpret_cast<EntryPoint>(
        library->FindSymbol("opencc_get_segmentation_plugin_v2"));
    const opencc_segmentation_plugin_v2* descriptor = entryPoint();
    if (descriptor == nullptr) {
      throw Exception("Plugin returned null descriptor.");
    }
    if (descriptor->struct_size < RequiredDescriptorSize()) {
      throw Exception("Plugin descriptor is too small.");
    }
    if (descriptor->abi_major != OPENCC_SEGMENTATION_PLUGIN_ABI_MAJOR) {
      throw Exception("Plugin ABI major version mismatch.");
    }
    if (descriptor->abi_minor < OPENCC_SEGMENTATION_PLUGIN_ABI_MINOR) {
      throw Exception("Plugin ABI minor version is older than required by the host.");
    }
    if (descriptor->segmentation_type == nullptr ||
        type != descriptor->segmentation_type) {
      throw Exception("Plugin type mismatch.");
    }
    std::shared_ptr<PluginLibrary> plugin(new PluginLibrary);
    plugin->library = library;
    plugin->descriptor = descriptor;
    return plugin;
  }

  std::mutex mutex_;
  std::unordered_map<std::string, std::shared_ptr<PluginLibrary>> cache_;
};

PluginManager& GetPluginManager() {
  static PluginManager manager;
  return manager;
}

} // namespace

SegmentationPtr CreatePluginSegmentation(const std::string& type,
                                         const PluginConfigPairs& configPairs) {
  // Allow explicit plugin library path via __plugin_library config pair,
  // bypassing search-based discovery (used by language bindings to bundle
  // plugins as optional packages with deterministic paths).
  std::string pluginLibraryPath;
  for (const auto& pair : configPairs) {
    if (pair.first == "__plugin_library") {
      pluginLibraryPath = pair.second;
      break;
    }
  }
  std::shared_ptr<PluginLibrary> plugin =
      pluginLibraryPath.empty()
          ? GetPluginManager().LoadByType(type)
          : GetPluginManager().LoadByPath(pluginLibraryPath, type);
  std::vector<opencc_kv_pair_t> rawConfig;
  rawConfig.reserve(configPairs.size());
  for (const auto& pair : configPairs) {
    rawConfig.push_back(
        {sizeof(opencc_kv_pair_t), pair.first.c_str(), pair.second.c_str()});
  }

  opencc_segmentation_handle_t* handle = nullptr;
  opencc_error_t error = {};
  error.struct_size = sizeof(error);
  opencc_segmentation_create_args_t args = {};
  args.struct_size = sizeof(args);
  args.config = rawConfig.empty() ? nullptr : rawConfig.data();
  args.config_size = rawConfig.size();
  args.out = &handle;
  args.error = &error;
  const int status = plugin->descriptor->create(&args);
  if (status != 0 || handle == nullptr) {
    const std::string message =
        TakeErrorMessage(plugin->descriptor, &error, "unknown error");
    throw InvalidFormat("Failed to initialize segmentation plugin '" + type +
                        "': " + message);
  }
  return SegmentationPtr(new PluginSegmentationAdapter(plugin, handle));
}

} // namespace opencc
