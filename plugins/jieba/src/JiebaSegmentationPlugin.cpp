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

#include "Exception.hpp"
#include "JiebaSegmentation.hpp"
#include "Segments.hpp"
#include "UTF8Util.hpp"
#include "plugin/OpenCCPlugin.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "cppjieba/Jieba.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
namespace {
std::wstring LocalWideFromUtf8(const std::string& utf8) {
  if (utf8.empty()) {
    return L"";
  }
  int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
  if (size <= 1) {
    return L"";
  }
  std::wstring wide(static_cast<size_t>(size), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wide[0], size);
  wide.resize(static_cast<size_t>(size - 1));
  return wide;
}
}  // namespace
#endif

struct opencc_segmentation_handle {
  explicit opencc_segmentation_handle(
      std::unique_ptr<opencc::Segmentation> segmentation)
      : segmentation(std::move(segmentation)) {}

  std::unique_ptr<opencc::Segmentation> segmentation;
};

namespace {

bool IsAbsolutePath(const std::string& path) {
  if (path.empty()) {
    return false;
  }
  if (path[0] == '/' || path[0] == '\\') {
    return true;
  }
  return path.size() > 1 && path[1] == ':';
}

bool IsReadableFile(const std::string& path) {
#if defined(_WIN32) || defined(_WIN64)
  const DWORD attributes = GetFileAttributesW(LocalWideFromUtf8(path).c_str());
  return attributes != INVALID_FILE_ATTRIBUTES &&
         (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
  std::ifstream ifs(path.c_str());
  return ifs.is_open();
#endif
}

std::string ParentDir(const std::string& path) {
  std::string normalized = path;
  while (!normalized.empty() &&
         (normalized.back() == '/' || normalized.back() == '\\')) {
    normalized.pop_back();
  }
  const std::string::size_type pos = normalized.find_last_of("/\\");
  if (pos == std::string::npos) {
    return "";
  }
  return normalized.substr(0, pos);
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

void SetError(opencc_error_t* error,
              int code,
              const std::string& message) {
  if (error == nullptr) {
    return;
  }
  if (error->message != nullptr) {
    delete[] error->message;
    error->message = nullptr;
  }
  error->code = code;
  char* buffer = new char[message.size() + 1];
  std::memcpy(buffer, message.c_str(), message.size() + 1);
  error->message = buffer;
}

bool ReadConfigValue(const opencc_kv_pair_t* config,
                     size_t size,
                     const char* key,
                     std::string* value) {
  for (size_t i = 0; i < size; i++) {
    if (config[i].key != nullptr && std::string(config[i].key) == key) {
      *value = config[i].value == nullptr ? "" : config[i].value;
      return true;
    }
  }
  return false;
}

uint32_t CodePointLength(const std::string& text) {
  return static_cast<uint32_t>(opencc::UTF8Util::Length(text.c_str()));
}

std::string ResolveResourcePath(const std::string& rawPath,
                                const std::string& configDir) {
  if (rawPath.empty()) {
    return rawPath;
  }
  if (IsAbsolutePath(rawPath) && IsReadableFile(rawPath)) {
    return rawPath;
  }
  if (IsReadableFile(rawPath)) {
    return rawPath;
  }
  if (!configDir.empty()) {
    const std::string candidate = JoinPath(configDir, rawPath);
    if (IsReadableFile(candidate)) {
      return candidate;
    }
    const std::string parent = ParentDir(configDir);
    if (!parent.empty()) {
      const std::string parentCandidate = JoinPath(parent, rawPath);
      if (IsReadableFile(parentCandidate)) {
        return parentCandidate;
      }
      const std::string parentParent = ParentDir(parent);
      if (!parentParent.empty()) {
        const std::string basename = rawPath.substr(rawPath.find_last_of("/\\") == std::string::npos
                                                        ? 0
                                                        : rawPath.find_last_of("/\\") + 1);
        const std::string devCandidate = JoinPath(parentParent, "deps/cppjieba/dict/" + basename);
        if (IsReadableFile(devCandidate)) {
          return devCandidate;
        }
      }
    }
  }
  const char* pluginDir = std::getenv("OPENCC_SEGMENTATION_PLUGIN_PATH");
  if (pluginDir != nullptr && *pluginDir != '\0') {
    const std::string candidate = JoinPath(pluginDir, rawPath);
    if (IsReadableFile(candidate)) {
      return candidate;
    }
  }
  const char* dataDir = std::getenv("OPENCC_DATA_DIR");
  if (dataDir != nullptr && *dataDir != '\0') {
    const std::string candidate = JoinPath(dataDir, rawPath);
    if (IsReadableFile(candidate)) {
      return candidate;
    }
  }
  if (!PACKAGE_DATA_DIRECTORY.empty()) {
    const std::string candidate = JoinPath(PACKAGE_DATA_DIRECTORY, rawPath);
    if (IsReadableFile(candidate)) {
      return candidate;
    }
  }


  return rawPath;
}

std::string ResolveAuxPath(const std::string& dictPath,
                           const std::string& modelPath,
                           const std::string& configDir,
                           const std::string& fileName) {
  const std::string::size_type pos = dictPath.find_last_of("/\\");
  if (pos != std::string::npos) {
    const std::string candidate = dictPath.substr(0, pos + 1) + fileName;
    if (IsReadableFile(candidate)) {
      return candidate;
    }
  }
  const std::string::size_type modelPos = modelPath.find_last_of("/\\");
  if (modelPos != std::string::npos) {
    const std::string modelCandidate =
        modelPath.substr(0, modelPos + 1) + fileName;
    if (IsReadableFile(modelCandidate)) {
      return modelCandidate;
    }
  }
  // Development fallback: try deps/cppjieba/dict/ relative to the source tree
  const std::string needle = "share/opencc/jieba_dict/";
  const std::string::size_type needlePos = dictPath.find(needle);
  if (needlePos != std::string::npos) {
    std::string alt = dictPath;
    alt.replace(needlePos, needle.size(), "plugins/jieba/deps/cppjieba/dict/");
    const std::string::size_type altPos = alt.find_last_of("/\\");
    if (altPos != std::string::npos) {
      const std::string altCandidate = alt.substr(0, altPos + 1) + fileName;
      if (IsReadableFile(altCandidate)) {
        return altCandidate;
      }
    }
  }
  return ResolveResourcePath("jieba_dict/" + fileName, configDir);
}

int CreateJiebaSegmentation(opencc_segmentation_create_args_t* args) {
  if (args == nullptr || args->struct_size < sizeof(*args) ||
      args->out == nullptr) {
    SetError(args == nullptr ? nullptr : args->error,
             OPENCC_ERROR_INVALID_ARGUMENT, "Output handle is null.");
    return -1;
  }
  *args->out = nullptr;

  std::string configDir;
  std::string dictPath;
  std::string modelPath;
  std::string userDictPath;
  ReadConfigValue(args->config, args->config_size, "__config_dir", &configDir);
  if (!ReadConfigValue(args->config, args->config_size, "dict_path", &dictPath) ||
      dictPath.empty()) {
    SetError(args->error, OPENCC_ERROR_PLUGIN_RESOURCE_MISSING,
             "Required resource missing: dict_path");
    return -1;
  }
  if (!ReadConfigValue(args->config, args->config_size, "model_path", &modelPath) ||
      modelPath.empty()) {
    SetError(args->error, OPENCC_ERROR_PLUGIN_RESOURCE_MISSING,
             "Required resource missing: model_path");
    return -1;
  }
  ReadConfigValue(args->config, args->config_size, "user_dict_path", &userDictPath);

  dictPath = ResolveResourcePath(dictPath, configDir);
  modelPath = ResolveResourcePath(modelPath, configDir);
  userDictPath = userDictPath.empty() ? "" : ResolveResourcePath(userDictPath, configDir);
  const std::string idfPath =
      ResolveAuxPath(dictPath, modelPath, configDir, "idf.utf8");
  const std::string stopWordsPath =
      ResolveAuxPath(dictPath, modelPath, configDir, "stop_words.utf8");

  if (!IsReadableFile(dictPath) || !IsReadableFile(modelPath) ||
      (!userDictPath.empty() && !IsReadableFile(userDictPath)) ||
      !IsReadableFile(idfPath) || !IsReadableFile(stopWordsPath)) {
    SetError(args->error, OPENCC_ERROR_PLUGIN_RESOURCE_MISSING,
             "Failed to locate Jieba resource files.");
    return -1;
  }

  try {
    std::unique_ptr<opencc::Segmentation> segmentation(
        new opencc::JiebaSegmentation(dictPath, modelPath, userDictPath));
    *args->out = new opencc_segmentation_handle(std::move(segmentation));
    return 0;
  } catch (const std::exception& ex) {
    SetError(args->error, OPENCC_ERROR_PLUGIN_RUNTIME_FAILURE, ex.what());
    return -1;
  } catch (...) {
    SetError(args->error, OPENCC_ERROR_PLUGIN_RUNTIME_FAILURE,
             "Unknown error while creating Jieba segmentation.");
    return -1;
  }
}

opencc::Segmentation* GetSegmentation(opencc_segmentation_handle_t* handle) {
  return handle->segmentation.get();
}

void FreeSegmentLengths(opencc_segment_length_array_t* segmentLengths) {
  if (segmentLengths == nullptr || segmentLengths->codepoint_lengths == nullptr) {
    return;
  }
  delete[] segmentLengths->codepoint_lengths;
  segmentLengths->codepoint_lengths = nullptr;
  segmentLengths->segment_count = 0;
}

int SegmentWithJieba(opencc_segmentation_segment_args_t* args) {
  if (args == nullptr || args->struct_size < sizeof(*args) ||
      args->handle == nullptr || args->segment_lengths == nullptr ||
      args->segment_lengths->struct_size < sizeof(*args->segment_lengths) ||
      args->utf8_text == nullptr) {
    SetError(args == nullptr ? nullptr : args->error,
             OPENCC_ERROR_INVALID_ARGUMENT,
             "Segment arguments are invalid.");
    return -1;
  }
  try {
    opencc::SegmentsPtr segments =
        GetSegmentation(args->handle)->Segment(args->utf8_text);
    const std::string text(args->utf8_text);
    std::vector<uint32_t> lengths;
    lengths.reserve(segments->Length() + 1);
    size_t byteOffset = 0;

    for (size_t i = 0; i < segments->Length(); i++) {
      const std::string token = segments->At(i);
      const size_t tokenPos = text.find(token, byteOffset);
      if (tokenPos == std::string::npos) {
        throw opencc::Exception("Jieba token sequence could not be aligned to input text.");
      }
      if (tokenPos > byteOffset) {
        lengths.push_back(CodePointLength(text.substr(byteOffset, tokenPos - byteOffset)));
      }
      lengths.push_back(CodePointLength(token));
      byteOffset = tokenPos + token.size();
    }
    if (byteOffset < text.size()) {
      lengths.push_back(CodePointLength(text.substr(byteOffset)));
    }

    args->segment_lengths->codepoint_lengths = nullptr;
    args->segment_lengths->segment_count = lengths.size();
    args->segment_lengths->codepoint_lengths =
        new uint32_t[args->segment_lengths->segment_count]();
    for (size_t i = 0; i < args->segment_lengths->segment_count; i++) {
      args->segment_lengths->codepoint_lengths[i] = lengths[i];
    }
    return 0;
  } catch (const std::exception& ex) {
    FreeSegmentLengths(args->segment_lengths);
    SetError(args->error, OPENCC_ERROR_PLUGIN_RUNTIME_FAILURE, ex.what());
    return -1;
  } catch (...) {
    FreeSegmentLengths(args->segment_lengths);
    SetError(args->error, OPENCC_ERROR_PLUGIN_RUNTIME_FAILURE,
             "Unknown error while segmenting text.");
    return -1;
  }
}

void DestroyJiebaSegmentation(opencc_segmentation_handle_t* handle) {
  delete handle;
}

void FreeError(opencc_error_t* error) {
  if (error == nullptr || error->message == nullptr) {
    return;
  }
  delete[] error->message;
  error->message = nullptr;
}

const opencc_segmentation_plugin_v2 kJiebaPlugin = {
    sizeof(opencc_segmentation_plugin_v2),
    OPENCC_SEGMENTATION_PLUGIN_ABI_MAJOR,
    OPENCC_SEGMENTATION_PLUGIN_ABI_MINOR,
    "opencc-jieba",
    "jieba",
    &CreateJiebaSegmentation,
    &SegmentWithJieba,
    &FreeSegmentLengths,
    &DestroyJiebaSegmentation,
    &FreeError,
};

} // namespace

extern "C" OPENCC_PLUGIN_EXPORT const opencc_segmentation_plugin_v2*
opencc_get_segmentation_plugin_v2(void) {
  return &kJiebaPlugin;
}
