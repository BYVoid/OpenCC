/**
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

#include "opencc.h"
#include "Config.hpp"
#include "ConversionChain.hpp"

using namespace Opencc;

struct InternalData {
  Config config;
  ConversionChainPtr conversionChain;
};

SimpleConverter::SimpleConverter(const std::string configFileName) {
  InternalData* data = new InternalData();
  internalData = data;
  data->config.LoadFile(configFileName);
  data->conversionChain = data->config.GetConversionChain();
}

SimpleConverter::~SimpleConverter() {
  delete (InternalData*)internalData;
}

std::string SimpleConverter::Convert(const std::string input) const {
  InternalData* data = (InternalData*)internalData;
  return data->conversionChain->Convert(input);
}

opencc_t opencc_new(const char* configFileName) {
  SimpleConverter* instance = new SimpleConverter(configFileName);
  return instance;
}

void opencc_delete(opencc_t opencc) {
  SimpleConverter* instance = reinterpret_cast<SimpleConverter*>(opencc);
  delete instance;
}

char* opencc_convert(opencc_t opencc, const char* input) {
  SimpleConverter* instance = reinterpret_cast<SimpleConverter*>(opencc);
  std::string converted = instance->Convert(input);
  char* output = new char[converted.length() + 1];
  strncpy(output, converted.c_str(), converted.length());
  return output;
}

void opencc_free_string(char* str) {
  delete[] str;
}
