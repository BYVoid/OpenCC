/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 Carbo Kuo <byvoid@byvoid.com>
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

#include "Config.hpp"
#include "Converter.hpp"
#include "UTF8Util.hpp"
#include "opencc.h"

#ifdef BAZEL
#include "tools/cpp/runfiles/runfiles.h"
using bazel::tools::cpp::runfiles::Runfiles;
#endif

using namespace opencc;

namespace {

struct InternalData {
  const ConverterPtr converter;

  InternalData(const ConverterPtr& _converter) : converter(_converter) {}

  static InternalData* NewInternalData(const std::string& configFileName,
                                       const std::vector<std::string>& paths,
                                       const char* argv0) {
    try {
      Config config;
#ifdef BAZEL
      std::string err;
      std::unique_ptr<Runfiles> bazel_runfiles(
          Runfiles::Create(argv0 != nullptr ? argv0 : "", &err));
      if (bazel_runfiles != nullptr) {
        std::vector<std::string> paths_with_runfiles = paths;
        paths_with_runfiles.push_back(
            bazel_runfiles->Rlocation("opencc~/data/config"));
        paths_with_runfiles.push_back(
            bazel_runfiles->Rlocation("opencc~/data/dictionary"));
        paths_with_runfiles.push_back(
            bazel_runfiles->Rlocation("_main/data/config"));
        paths_with_runfiles.push_back(
            bazel_runfiles->Rlocation("_main/data/dictionary"));
        return new InternalData(
            config.NewFromFile(configFileName, paths_with_runfiles, argv0));
      }
#endif
      return new InternalData(config.NewFromFile(configFileName, paths, argv0));
    } catch (Exception& ex) {
      throw std::runtime_error(ex.what());
    }
  }
};

} // namespace

SimpleConverter::SimpleConverter(const std::string& configFileName)
    : SimpleConverter(configFileName, std::vector<std::string>()) {}

SimpleConverter::SimpleConverter(const std::string& configFileName,
                                 const std::vector<std::string>& paths)
    : SimpleConverter(configFileName, paths, nullptr) {}

SimpleConverter::SimpleConverter(const std::string& configFileName,
                                 const std::vector<std::string>& paths,
                                 const char* argv0)
    : internalData(
          InternalData::NewInternalData(configFileName, paths, argv0)) {}

SimpleConverter::~SimpleConverter() { delete (InternalData*)internalData; }

std::string SimpleConverter::Convert(const std::string& input) const {
  try {
    const InternalData* data = (InternalData*)internalData;
    return data->converter->Convert(input);
  } catch (Exception& ex) {
    throw std::runtime_error(ex.what());
  }
}

std::string SimpleConverter::Convert(const char* input) const {
  return Convert(std::string(input));
}

std::string SimpleConverter::Convert(const char* input, size_t length) const {
  if (length == static_cast<size_t>(-1)) {
    return Convert(std::string(input));
  } else {
    return Convert(UTF8Util::FromSubstr(input, length));
  }
}

size_t SimpleConverter::Convert(const char* input, char* output) const {
  try {
    const InternalData* data = (InternalData*)internalData;
    return data->converter->Convert(input, output);
  } catch (Exception& ex) {
    throw std::runtime_error(ex.what());
  }
}

size_t SimpleConverter::Convert(const char* input, size_t length,
                                char* output) const {
  if (length == static_cast<size_t>(-1)) {
    return Convert(input, output);
  } else {
    std::string trimmed = UTF8Util::FromSubstr(input, length);
    return Convert(trimmed.c_str(), output);
  }
}

static std::string cError;

opencc_t opencc_open_internal(const char* configFileName) {
  try {
    if (configFileName == nullptr) {
      configFileName = OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD;
    }
    SimpleConverter* instance = new SimpleConverter(configFileName);
    return instance;
  } catch (std::runtime_error& ex) {
    cError = ex.what();
    return reinterpret_cast<opencc_t>(-1);
  }
}

#ifdef _MSC_VER
opencc_t opencc_open_w(const wchar_t* configFileName) {
  try {
    if (configFileName == nullptr) {
      return opencc_open_internal(nullptr);
    }
    std::string utf8fn = UTF8Util::U16ToU8(configFileName);
    return opencc_open_internal(utf8fn.c_str());
  } catch (std::runtime_error& ex) {
    cError = ex.what();
    return reinterpret_cast<opencc_t>(-1);
  }
}
opencc_t opencc_open(const char* configFileName) {
  if (configFileName == nullptr) {
    return opencc_open_internal(nullptr);
  }
  std::wstring wFileName;
  int convcnt = MultiByteToWideChar(CP_ACP, 0, configFileName, -1, NULL, 0);
  if (convcnt > 0) {
    wFileName.resize(convcnt);
    MultiByteToWideChar(CP_ACP, 0, configFileName, -1, &wFileName[0], convcnt);
  }
  return opencc_open_w(wFileName.c_str());
}
#else
opencc_t opencc_open(const char* configFileName) {
  return opencc_open_internal(configFileName);
}
#endif

int opencc_close(opencc_t opencc) {
  SimpleConverter* instance = reinterpret_cast<SimpleConverter*>(opencc);
  delete instance;
  return 0;
}

size_t opencc_convert_utf8_to_buffer(opencc_t opencc, const char* input,
                                     size_t length, char* output) {
  try {
    SimpleConverter* instance = reinterpret_cast<SimpleConverter*>(opencc);
    return instance->Convert(input, length, output);
  } catch (std::runtime_error& ex) {
    cError = ex.what();
    return static_cast<size_t>(-1);
  }
}

char* opencc_convert_utf8(opencc_t opencc, const char* input, size_t length) {
  try {
    SimpleConverter* instance = reinterpret_cast<SimpleConverter*>(opencc);
    std::string converted = instance->Convert(input, length);
    char* output = new char[converted.length() + 1];
    strncpy(output, converted.c_str(), converted.length());
    output[converted.length()] = '\0';
    return output;
  } catch (std::runtime_error& ex) {
    cError = ex.what();
    return nullptr;
  }
}

void opencc_convert_utf8_free(char* str) { delete[] str; }

const char* opencc_error(void) { return cError.c_str(); }
