/*
 * Open Chinese Convert
 *
 * Copyright 2010-2017 BYVoid <byvoid@byvoid.com>
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

#include "DartsDict.hpp"
#include "DictConverter.hpp"
#include "TextDict.hpp"

using namespace opencc;

DictPtr LoadDictionary(const string& format, const string& inputFileName) {
  if (format == "text") {
    return SerializableDict::NewFromFile<TextDict>(inputFileName);
  } else if (format == "ocd") {
    return SerializableDict::NewFromFile<DartsDict>(inputFileName);
  } else {
    fprintf(stderr, "Unknown dictionary format: %s\n", format.c_str());
    exit(2);
  }
  return nullptr;
}

SerializableDictPtr ConvertDict(const string& format,
                                      const DictPtr dict) {
  if (format == "text") {
    return TextDict::NewFromDict(*dict.get());
  } else if (format == "ocd") {
    return DartsDict::NewFromDict(*dict.get());
  } else {
    fprintf(stderr, "Unknown dictionary format: %s\n", format.c_str());
    exit(2);
  }
  return nullptr;
}

namespace opencc {
void ConvertDictionary(const string inputFileName, const string outputFileName,
                       const string formatFrom, const string formatTo) {
  DictPtr dictFrom = LoadDictionary(formatFrom, inputFileName);
  SerializableDictPtr dictTo = ConvertDict(formatTo, dictFrom);
  dictTo->SerializeToFile(outputFileName);
}
}
