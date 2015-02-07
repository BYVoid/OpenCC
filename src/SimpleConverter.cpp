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
#include "UTF8Util.hpp"

using namespace opencc;

struct InternalData {
  const ConverterPtr converter;

  InternalData(const ConverterPtr& _converter) : converter(_converter) {}
};

SimpleConverter::SimpleConverter(const std::string& configFileName) {
  try {
    Config config;
    internalData = new InternalData(config.NewFromFile(configFileName));
  } catch (Exception& ex) {
    throw std::runtime_error(ex.what());
  }
}

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
  return Convert(string(input));
}

std::string SimpleConverter::Convert(const char* input, size_t length) const {
  if (length == static_cast<size_t>(-1)) {
    return Convert(string(input));
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
    string trimmed = UTF8Util::FromSubstr(input, length);
    return Convert(trimmed.c_str(), output);
  }
}

static string cError;

opencc_t opencc_open(const char* configFileName) {
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

int opencc_close(opencc_t opencc) {
  try {
    SimpleConverter* instance = reinterpret_cast<SimpleConverter*>(opencc);
    delete instance;
    return 0;
  } catch (std::exception& ex) {
    cError = ex.what();
    return 1;
  }
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
