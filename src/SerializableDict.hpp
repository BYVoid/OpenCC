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

#pragma once

#include "Dict.hpp"
#if defined(_WIN32) || defined(_WIN64)
#include "WinUtil.hpp"
#endif

namespace opencc {
/**
 * Serializable dictionary interface
 * @ingroup opencc_cpp_api
 */
class OPENCC_EXPORT SerializableDict {
public:
  /**
   * Serializes the dictionary and writes in to a file.
   */
  virtual void SerializeToFile(FILE* fp) const = 0;

  /**
   * Serializes the dictionary and writes in to a file.
   */
  virtual void SerializeToFile(const std::string& fileName) const {
#if defined(_WIN32) || defined(_WIN64)
    FILE* fp = _wfopen(internal::WideFromUtf8(fileName).c_str(), L"wb");
#else
    FILE* fp = fopen(fileName.c_str(), "wb");
#endif
    if (fp == NULL) {
      throw FileNotWritable(fileName);
    }
    SerializeToFile(fp);
    fclose(fp);
  }

  template <typename DICT>
  static bool TryLoadFromFile(const std::string& fileName,
                              std::shared_ptr<DICT>* dict) {
    FILE* fp = nullptr;
#if defined(_WIN32) || defined(_WIN64)
    fp = _wfopen(internal::WideFromUtf8(fileName).c_str(), L"rb");
#else
    fp = fopen(fileName.c_str(), "rb");
#endif

    if (fp == NULL) {
      return false;
    }
    std::shared_ptr<DICT> loadedDict = DICT::NewFromFile(fp);
    fclose(fp);
    *dict = loadedDict;
    return true;
  }

  template <typename DICT>
  static std::shared_ptr<DICT> NewFromFile(const std::string& fileName) {
    std::shared_ptr<DICT> dict;
    if (!TryLoadFromFile<DICT>(fileName, &dict)) {
      throw FileNotFound(fileName);
    }
    return dict;
  }
};
} // namespace opencc
