/*
 * Open Chinese Convert
 *
 * Copyright 2010-2013 BYVoid <byvoid@byvoid.com>
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
  class SerializableDict : public Dict {
    public:
      virtual void LoadFromFile(FILE* fp) = 0;
      virtual void SerializeToFile(FILE* fp) = 0;
      virtual bool TryLoadFromFile(const string fileName) {
        FILE* fp = fopen(fileName.c_str(), "rb");
        if (fp == NULL) {
          return false;
        }
        LoadFromFile(fp);
        fclose(fp);
        return true;
      }

      virtual void LoadFromFile(const string fileName) {
        if (!TryLoadFromFile(fileName)) {
          throw FileNotFound(fileName);
        }
      }

      virtual void SerializeToFile(const string fileName) {
        FILE* fp = fopen(fileName.c_str(), "wb");
        if (fp == NULL) {
          throw FileNotWritable(fileName);
        }
        SerializeToFile(fp);
        fclose(fp);
      }
  };
}
