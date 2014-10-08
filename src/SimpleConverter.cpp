/*
 * Open Chinese Convert
 *
 * Copyright 2010-2014 BYVoid <byvoid@byvoid.com>
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
#include "opencc.h"

using namespace opencc;

struct InternalData {
  const ConverterPtr converter;

  InternalData(const ConverterPtr& _converter) : converter(_converter) {
  }
};

SimpleConverter::SimpleConverter(const std::string& configFileName) {
  try {
    Config config;
    internalData = new InternalData(config.NewFromFile(configFileName));
  } catch (Exception& ex) {
    throw std::runtime_error(ex.what());
  }
}

SimpleConverter::~SimpleConverter() {
  delete (InternalData*)internalData;
}

std::string SimpleConverter::Convert(const std::string& input) const {
  try {
    const InternalData* data = (InternalData*)internalData;
    return data->converter->Convert(input);
  } catch (Exception& ex) {
    throw std::runtime_error(ex.what());
  }
}

opencc_t opencc_new(const char* configFileName) {
  try {
    SimpleConverter* instance = new SimpleConverter(configFileName);
    return instance;
  } catch (std::runtime_error& ex) {
    // TODO report error
    return NULL;
  }
}

void opencc_delete(opencc_t opencc) {
  SimpleConverter* instance = reinterpret_cast<SimpleConverter*>(opencc);
  delete instance;
}

char* opencc_convert(opencc_t opencc, const char* input) {
  try {
    SimpleConverter* instance = reinterpret_cast<SimpleConverter*>(opencc);
    std::string converted = instance->Convert(input);
    char* output = new char[converted.length() + 1];

    strncpy(output, converted.c_str(), converted.length());
    output[converted.length()] = '\0';
    return output;
  } catch (std::runtime_error& ex) {
    // TODO report error
    return NULL;
  }
}

void opencc_free_string(char* str) {
  delete[] str;
}
