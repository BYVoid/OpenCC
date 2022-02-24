/*
 * Open Chinese Convert
 *
 * Copyright 2010-2020 Carbo Kuo <byvoid@byvoid.com>
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

#include "DictConverter.hpp"
#include "MarisaDict.hpp"
#include "TextDict.hpp"

#ifdef ENABLE_DARTS
#include "DartsDict.hpp"
#endif

using namespace opencc;

DictPtr LoadDictionary(const std::string& format,
                       const std::string& inputFileName) {
  if (format == "text") {
    return SerializableDict::NewFromFile<TextDict>(inputFileName);
  } else if (format == "ocd") {
#ifdef ENABLE_DARTS
    return SerializableDict::NewFromFile<DartsDict>(inputFileName);
#endif
  } else if (format == "ocd2") {
    return SerializableDict::NewFromFile<MarisaDict>(inputFileName);
  }
  fprintf(stderr, "Unknown dictionary format: %s\n", format.c_str());
  exit(2);
  return nullptr;
}

SerializableDictPtr ConvertDict(const std::string& format, const DictPtr dict) {
  if (format == "text") {
    return TextDict::NewFromDict(*dict.get());
  } else if (format == "ocd") {
#ifdef ENABLE_DARTS
    return DartsDict::NewFromDict(*dict.get());
#endif
  } else if (format == "ocd2") {
    return MarisaDict::NewFromDict(*dict.get());
  }
  fprintf(stderr, "Unknown dictionary format: %s\n", format.c_str());
  exit(2);
  return nullptr;
}

namespace opencc {
void ConvertDictionary(const std::string& inputFileName,
                       const std::string& outputFileName,
                       const std::string& formatFrom,
                       const std::string& formatTo) {
  DictPtr dictFrom = LoadDictionary(formatFrom, inputFileName);
  SerializableDictPtr dictTo = ConvertDict(formatTo, dictFrom);
  dictTo->SerializeToFile(outputFileName);
}
} // namespace opencc
