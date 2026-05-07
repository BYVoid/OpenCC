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
    FILE* fp = fopen(fileName.c_str(), "wb");
    if (fp == NULL) {
      throw FileNotWritable(fileName);
    }
    SerializeToFile(fp);
    fclose(fp);
  }

  template <typename DICT>
  static bool TryLoadFromFile(const std::string& fileName,
                              std::shared_ptr<DICT>* dict) {
    FILE* fp =
#ifdef _MSC_VER
        // well, the 'GetPlatformString' shall return a 'wstring'
        _wfopen(UTF8Util::GetPlatformString(fileName).c_str(), L"rb")
#else
        fopen(UTF8Util::GetPlatformString(fileName).c_str(), "rb")
#endif // _MSC_VER
        ;

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
